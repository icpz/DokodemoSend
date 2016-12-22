
#include "DSPacket.h"
#include <QDebug>
#include <libnet.h>

DSArpPacket::DSArpPacket(const QString &device, int ipFamily,
                const QString &srcIp, const QString &dstIp)
    : DSPacket(device, srcIp, dstIp, ipFamily, ETHERTYPE_ARP){

}

void DSArpPacket::setupParameter(const QVector<uint8_t> &srcMac,
            const QVector<uint8_t> &dstMac, uint16_t type, const QVector<uint8_t> &payload) {

    ready = false;
    this->srcMac = srcMac;
    this->dstMac = dstMac;
    this->type = type;
    this->payload = payload;

    updateParameter();
    ready = true;
}

void DSArpPacket::updateParameter() {
    libnet_ptag_t ptag;
    static const QVector<uint8_t> broadcast(6, 0xff);

    DSPacket::initLinkHandle();

    uint32_t srcIp = libnet_name2addr4(handle,
                     const_cast<char *>(source.toStdString().c_str()), LIBNET_DONT_RESOLVE);
    uint32_t dstIp = libnet_name2addr4(handle,
                     const_cast<char *>(destination.toStdString().c_str()), LIBNET_DONT_RESOLVE);

    qDebug() << "building arp...";
    ptag = libnet_build_arp(ARPHRD_ETHER, ETHERTYPE_IP, 6, 4,
                            type, srcMac.constData(),
                            reinterpret_cast<uint8_t *>(&srcIp),
                            dstMac.constData(),
                            reinterpret_cast<uint8_t *>(&dstIp),
                            payload.constData(), payload.size(), handle, 0);

    if (ptag == -1) qDebug() << "build arp failed";

    DSPacket::updateLinkHeader(srcMac, broadcast, ETHERTYPE_ARP);
    DSPacket::updatePacketData();
}

