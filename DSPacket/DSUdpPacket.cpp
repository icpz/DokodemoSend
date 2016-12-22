#include "DSPacket.h"
#include <QDebug>

DSUdpPacket::DSUdpPacket(const QString &device, int ipFamily, const QString &srcIp, const QString &dstIp)
    : DSPacket(device, srcIp, dstIp, ipFamily, IPPROTO_UDP) {

}

void DSUdpPacket::setupParameter(uint16_t srcPort, uint16_t dstPort, const QVector<uint8_t> &payload) {

    ready = false;
    this->srcPort = srcPort;
    this->dstPort = dstPort;
    this->payload = payload;

    updateParameter();
    ready = true;
}

void DSUdpPacket::updateParameter() {
    libnet_ptag_t ptag;
    int udpPacketLength = LIBNET_UDP_H + payload.size();

    DSPacket::initIpHandle();

    ptag = libnet_build_udp(srcPort, dstPort, udpPacketLength, 0,
                payload.constData(), payload.size(), handle, 0);

    DSPacket::updateIpHeader(udpPacketLength);
    DSPacket::updatePacketData();
}


