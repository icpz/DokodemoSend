#include "DSPacket.h"
#include <QDebug>

DSTcpPacket::DSTcpPacket(const QString &device, int ipFamily, const QString &srcIp, const QString &dstIp)
    : DSPacket(device, srcIp, dstIp, ipFamily, IPPROTO_TCP) {

}

void DSTcpPacket::setupParameter(uint16_t srcPort, uint16_t dstPort,
        int seq, int ack, int flag, int windowsize, uint16_t urgpoint,
        const QVector<uint8_t> &option, const QVector<uint8_t> &payload) {

    ready = false;
    this->srcPort = srcPort;
    this->dstPort = dstPort;
    this->seq = seq;
    this->ack = ack;
    this->flag = flag;
    this->windowsize = windowsize;
    this->urgpoint = urgpoint;
    this->option = option;
    this->payload = payload;

    updateParameter();
    ready = true;

}

void DSTcpPacket::updateParameter() {
    libnet_ptag_t ptag;
    int tcpPacketLength = LIBNET_TCP_H + option.size() + payload.size();

    DSPacket::initIpHandle();

    if (!option.empty()) {
        ptag = libnet_build_tcp_options(option.constData(), option.size(),
                handle, 0);
    }

    ptag = libnet_build_tcp(srcPort, dstPort, seq, ack, flag, windowsize, 0,
            urgpoint, tcpPacketLength,
            payload.constData(), payload.size(), handle, 0);

    DSPacket::updateIpHeader(tcpPacketLength);
    DSPacket::updatePacketData();
}




