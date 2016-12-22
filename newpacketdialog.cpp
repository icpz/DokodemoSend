#include "newpacketdialog.h"
#include "ui_newpacketdialog.h"
#include <pcap.h>
#include <QDebug>
#include <QCompleter>
#include <sys/socket.h>

NewPacketDialog::NewPacketDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewPacketDialog),
    devicelist(0)
{
    ui->setupUi(this);

    initDeviceList();
    initSignals();

}

NewPacketDialog::~NewPacketDialog()
{
    delete ui;
}

void NewPacketDialog::initDeviceList() {
    pcap_if_t *alldev;
    char errbuf[PCAP_ERRBUF_SIZE];
    char ipBuf[INET6_ADDRSTRLEN];
    QStringList devList;

    if (pcap_findalldevs(&alldev, errbuf)) {
        qDebug() << errbuf << endl;
        QApplication::quit();
    }

    for (auto device = alldev; device; device = device->next) {
        DSDevice dev(tr(device->name));
        for (auto addr = device->addresses; addr; addr = addr->next) {
            auto ipFamily = addr->addr->sa_family;
            if(ipFamily == AF_INET || ipFamily == AF_INET6) {
                inet_ntop(ipFamily, get_in_addr((struct sockaddr *)addr->addr), ipBuf, sizeof ipBuf);
                dev.push_address(ipFamily, ipBuf);
            }
        }
        if (dev.address_count()) {
            devicelist.push_back(dev);
        }
    }

    for (const auto &c : devicelist) {
        devList << c.get_device_name();
    }
    ui->captureComboBox->addItems(devList);

    pcap_freealldevs(alldev);
}

void NewPacketDialog::initSignals() {
    connect(ui->captureComboBox,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            [&](const QString &dev) {
                int ipFamily = this->getIpFamily();
                updateSourceIpCompleter(ipFamily, dev);
            });

    connect(ui->icmpIDAndSeqMode, &QRadioButton::clicked,
            [&](bool) {
                ui->icmpHexMode->setChecked(false);
                ui->icmpLabelID->setText(tr("ID"));
                ui->icmpLabelSEQ->show();
                ui->icmpSeqEdit->show();
            });
    connect(ui->icmpHexMode, &QRadioButton::clicked,
            [&](bool) {
                ui->icmpIDAndSeqMode->setChecked(false);
                ui->icmpLabelID->setText(tr("Hex"));
                ui->icmpLabelSEQ->hide();
                ui->icmpSeqEdit->hide();
            });
}

void NewPacketDialog::show() {

    QDialog::show();
}

void NewPacketDialog::updateSourceIpCompleter(int family, const QString &dev) {
    int devIndex = getDeviceByName(dev);
    if (devIndex == -1) return;
    QStringList completerList(devicelist[getDeviceByName(dev)].get_ip_addresses(family));
    auto newCompleter = new QCompleter(completerList, this);

    newCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    ui->srcIPEdit->setCompleter(newCompleter);
}

QVector<uint8_t> NewPacketDialog::parse_hex(const QString &hexes) {
    QVector<uint8_t> result;
    QString filtered;

    std::remove_copy_if(std::cbegin(hexes), std::cend(hexes),
                    std::back_inserter(filtered), [](QChar c) {
                        return c == QChar(' ')
                            || c == QChar('\n')
                            || c == QChar('-');
                    });
    for (int i = 0; i < filtered.size(); i += 2) {
        result.push_back(filtered.mid(i, 2).toUInt(nullptr, 16));
    }
    return result;
}
