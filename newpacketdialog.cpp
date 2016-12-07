#include "newpacketdialog.h"
#include "ui_newpacketdialog.h"
#include <pcap.h>
#include <QDebug>
#include <QCompleter>
#include <libnet.h>
#include <sys/socket.h>

NewPacketDialog::NewPacketDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewPacketDialog),
    devicelist(0)
{
    ui->setupUi(this);

    initDeviceList();

    connect(ui->captureComboBox,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            [&](const QString &dev) {
                int ipFamily = get_ip_family(ui->ipModeComboBox->currentText());
                updateSourceIpCompleter(ipFamily, dev);
            });
    connect(ui->ipModeComboBox, &QComboBox::currentTextChanged,
            [&](const QString &ipMode) {
                int ipFamily = get_ip_family(ipMode);
                updateSourceIpCompleter(ipFamily, ui->captureComboBox->currentText());
            });
}

NewPacketDialog::~NewPacketDialog()
{
    delete ui;
}

std::string NewPacketDialog::getNewPacket() {
    return "in packet";
}

void *NewPacketDialog::get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void NewPacketDialog::initDeviceList() {
    pcap_if_t *alldev;
    char errbuf[std::max(PCAP_ERRBUF_SIZE, LIBNET_ERRBUF_SIZE)];
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
                qDebug() << device->name << ipBuf;
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

void NewPacketDialog::show() {

    QDialog::show();
}

int NewPacketDialog::getDeviceByName(const QString &name) {
    for (int i = 0; i < devicelist.size(); ++i) {
        if (devicelist[i].get_device_name() == name)
            return i;
    }
    return -1;
}

int NewPacketDialog::get_ip_family(const QString &af) {
    return af.indexOf(tr("4")) == -1 ? AF_INET6 : AF_INET;
}

void NewPacketDialog::updateSourceIpCompleter(int family, const QString &dev) {
    QStringList completerList(devicelist[getDeviceByName(dev)].get_ip_addresses(family));
    auto newCompleter = new QCompleter(completerList, this);

    newCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    ui->srcIPEdit->setCompleter(newCompleter);
}
