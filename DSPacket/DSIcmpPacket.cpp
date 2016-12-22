#include "DSPacket.h"
#include <QDebug>
#include <libnet.h>
#include <algorithm>

DSIcmpPacket::DSIcmpPacket(const QString &device, int ipFamily,
                           const QString &srcIp, const QString &dstIp)
        : DSPacket(device, srcIp, dstIp, ipFamily, IPPROTO_ICMP) {

}

void DSIcmpPacket::setupParameter(uint8_t type, uint8_t code, uint32_t rest,
                                  const QVector<uint8_t> &payload) {

    ready = false;
    this->type = type;
    this->code = code;
    this->rest = rest;
    this->payload = payload;

    updateParameter();
    ready = true;
}

struct __icmp_min_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t rest;
};

uint16_t __checksum(uint16_t *buffer,uint32_t size) {
    uint32_t cksum = 0;

    while (size > 1) {
        cksum += *buffer++;
        size -= sizeof(uint16_t);
    }

    if (size) {
        cksum += *(uint8_t *)buffer;
    }

    while (cksum >> 16)
        cksum = (cksum >> 16) + (cksum & 0xffff);
    return static_cast<uint16_t>(~cksum);
}

void DSIcmpPacket::updateParameter() {
    uint8_t *icmpBuf;
    int icmpPacketLength = sizeof(__icmp_min_header) + payload.size();

    DSPacket::initIpHandle();

    qDebug() << "building icmp packet...";
    icmpBuf = new uint8_t[icmpPacketLength];
    if (!icmpBuf) qDebug() << "bad alloc";

    __icmp_min_header *icmp_hdr = reinterpret_cast<__icmp_min_header *>(icmpBuf);
    icmp_hdr->type = type;
    icmp_hdr->code = code;
    icmp_hdr->checksum = 0;
    icmp_hdr->rest = rest;
    std::copy(payload.begin(), payload.end(), icmpBuf + sizeof(__icmp_min_header));
    icmp_hdr->checksum = __checksum(reinterpret_cast<uint16_t *>(icmpBuf), icmpPacketLength);

    DSPacket::updateIpHeader(0, icmpBuf, icmpPacketLength);
    DSPacket::updatePacketData();
    delete []icmpBuf;
}
