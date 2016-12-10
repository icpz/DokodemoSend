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

DSPacket *NewPacketDialog::getNewPacket() {
    DSPacket *result = nullptr;
    QString &&type = ui->tabPacketType->tabText(ui->tabPacketType->currentIndex());

    if (type == tr("TCP")) {
        result = generateTcpPacket();
    } else if (type == tr("UDP")) {
        result = generateUdpPacket();
    } else if (type == tr("IP")) {
        result = generateIpPacket();
    }

    return result;
}

void *NewPacketDialog::get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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
                // qDebug() << device->name << ipBuf;
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

DSPacket *NewPacketDialog::generateTcpPacket() const {
    if (!checkTcp()) return nullptr;

    qDebug() << "TCP Packet generating...";
    QString &&dev = ui->captureComboBox->currentText();
    auto *result = new DSTcpPacket(dev, get_ip_family(ui->ipModeComboBox->currentText()),
                                    ui->srcIPEdit->text(), ui->dstIPEdit->text());
    result->setupParameter(ui->tcpSrcPort->text().toShort(), ui->tcpDstPort->text().toShort(),
                           ui->tcpSeqNumber->text().toInt(), ui->tcpAckNumber->text().toInt(),
                           getTcpFlag(), ui->tcpWindow->text().toInt(), ui->tcpUrgent->text().toShort(),
                           parse_hex(ui->tcpOptions->text()), parse_hex(ui->packetPayload->toPlainText()));

    return result;
}

DSPacket *NewPacketDialog::generateUdpPacket() const {
    if (!checkUdp()) return nullptr;

    qDebug() << "UDP Packet generating...";
    QString &&dev = ui->captureComboBox->currentText();
    auto *result = new DSUdpPacket(dev, get_ip_family(ui->ipModeComboBox->currentText()),
                                   ui->srcIPEdit->text(), ui->dstIPEdit->text());
    result->setupParameter(ui->udpSrcPort->text().toShort(), ui->udpDstPort->text().toShort(),
                           parse_hex(ui->packetPayload->toPlainText()));

    return result;
}

DSPacket *NewPacketDialog::generateIpPacket() const {
    return nullptr;
}

int NewPacketDialog::getTcpFlag() const {
    int result = 0;

    if (ui->tcpFIN->isChecked()) result |= TH_FIN;
    if (ui->tcpSYN->isChecked()) result |= TH_SYN;
    if (ui->tcpACK->isChecked()) result |= TH_ACK;
    if (ui->tcpURG->isChecked()) result |= TH_URG;
    if (ui->tcpPSH->isChecked()) result |= TH_PUSH;
    if (ui->tcpRST->isChecked()) result |= TH_RST;

    return result;
}

bool NewPacketDialog::checkTcp() const {
    return true;
}

bool NewPacketDialog::checkUdp() const {
    return true;
}

QVector<uint8_t> NewPacketDialog::parse_hex(const QString &hexes) {
    QVector<uint8_t> result;
    QString filtered;

    std::remove_copy(std::cbegin(hexes), std::cend(hexes),
                     std::back_inserter(filtered), QChar(' '));
    for (int i = 0; i < filtered.size(); i += 2) {
        result.push_back(filtered.mid(i, 2).toUInt(nullptr, 16));
    }
    return result;
}
