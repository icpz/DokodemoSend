#include "newpacketdialog.h"
#include <QDebug>

DSPacket *NewPacketDialog::generateTcpPacket() const {
    if (!checkTcp()) return nullptr;

    qDebug() << "TCP Packet generating...";

    auto *result = new DSTcpPacket(getDevice(), getIpFamily(),
                                   getSourceIp(), getDestinationIp());
    result->setTTL(getTTL());
    result->setDelay(getDelay());
    result->setupParameter(ui->tcpSrcPort->text().toShort(), ui->tcpDstPort->text().toShort(),
                           ui->tcpSeqNumber->text().toInt(), ui->tcpAckNumber->text().toInt(),
                           getTcpFlag(), ui->tcpWindow->text().toInt(), ui->tcpUrgent->text().toShort(),
                           parse_hex(ui->tcpOptions->text()), getPayload());
    return result;
}

DSPacket *NewPacketDialog::generateUdpPacket() const {
    if (!checkUdp()) return nullptr;

    qDebug() << "UDP Packet generating...";

    auto *result = new DSUdpPacket(getDevice(), getIpFamily(),
                                   getSourceIp(), getDestinationIp());

    result->setTTL(getTTL());
    result->setDelay(getDelay());

    result->setupParameter(ui->udpSrcPort->text().toShort(), ui->udpDstPort->text().toShort(),
                           getPayload());
    return result;
}

DSPacket *NewPacketDialog::generateIpPacket() const {
    if (!checkIp()) return nullptr;
    DSPacket *result = nullptr;
    int ipFamily = getIpFamily();

    qDebug() << "RAW Ip Packet generating...";

    if (ipFamily == AF_INET) {
        auto ipv4 = new DSIpPacket4(getDevice(), getSourceIp(), getDestinationIp(),
                                    ui->ipProto->text().toInt());

        ipv4->setTTL(getTTL());
        ipv4->setDelay(getDelay());

        ipv4->setupParameter(ui->ipTos->text().toInt(nullptr, 16),
                             ui->ipIdentifier->text().toInt(),
                             getIpFlag(),
                             ui->ipFrag->text().toInt(),
                             parse_hex(ui->ipOptions->text()),
                             getPayload());
        result = ipv4;
    }
    return result;
}

DSPacket *NewPacketDialog::generateArpPacket() const {
    if (!checkArp()) return nullptr;
    DSArpPacket *result = nullptr;

    static const QVector<uint8_t> requestAddress(6, 0x00);

    result = new DSArpPacket(getDevice(), getIpFamily(), getSourceIp(),
                             getDestinationIp());

    result->setTTL(getTTL());
    result->setDelay(getDelay());

    result->setupParameter(parse_hex(ui->srcMACEdit->text()),
                           getArpType() == ARPOP_REQUEST ? requestAddress : parse_hex(ui->dstMACEdit->text()),
                           getArpType(), getPayload());

    return result;
}

DSPacket *NewPacketDialog::generateIcmpPacket() const {
    if (!checkIcmp()) return nullptr;
    DSIcmpPacket *result = nullptr;

    uint32_t rest;
    if (ui->icmpHexMode->isChecked()) {
        auto hexBuf = parse_hex(ui->icmpIDEdit->text());
        for (auto c : hexBuf) rest = (rest << 8) + c;
    } else {
        rest = (ui->icmpIDEdit->text().toInt())
             + (ui->icmpSeqEdit->text().toInt() << 16);
    }
    result = new DSIcmpPacket(getDevice(), getIpFamily(), getSourceIp(),
                              getDestinationIp());

    result->setTTL(getTTL());
    result->setDelay(getDelay());

    result->setupParameter(ui->icmpTypeEdit->text().toInt(),
                           ui->icmpCodeEdit->text().toInt(),
                           rest, getPayload());

    return result;
}

