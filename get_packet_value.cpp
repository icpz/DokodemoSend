#include "newpacketdialog.h"

DSPacket *NewPacketDialog::getNewPacket() {
    DSPacket *result = nullptr;
    QString &&type = ui->tabPacketType->tabText(ui->tabPacketType->currentIndex());

    if (type == tr("TCP")){
        result = generateTcpPacket();
    } else if (type == tr("UDP")) {
        result = generateUdpPacket();
    } else if (type == tr("IPv4") || type == tr("IPv6")) {
        result = generateIpPacket();
    } else if (type == tr("ARP")) {
        result = generateArpPacket();
    } else if (type == tr("ICMP")) {
        result = generateIcmpPacket();
    }

    return result;
}

void *NewPacketDialog::get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int NewPacketDialog::getDeviceByName(const QString &name) const {
    for (int i = 0; i < devicelist.size(); ++i) {
        if (devicelist[i].get_device_name() == name)
            return i;
    }
    return -1;
}

int NewPacketDialog::getIpFamily() const {
    return AF_INET;
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

int NewPacketDialog::getIpFlag() const {
    int result = 0;

    if (ui->ipDF->isChecked()) result |= IP_DF;
    if (ui->ipMF->isChecked()) result = IP_MF;

    return result;
}

int NewPacketDialog::getArpType() const {
    int result = 0;

    if (ui->arpRequest->isChecked()) result = ARPOP_REQUEST;
    else if (ui->arpReply->isChecked()) result = ARPOP_REPLY;

    return result;
}
