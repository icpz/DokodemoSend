#include "DSPacket.h"
#include <QDebug>

DSIpPacket4::DSIpPacket4(const QString &device,
                        const QString &srcIp, const QString &dstIp, uint8_t proto)
    : DSPacket(device, srcIp, dstIp, AF_INET, proto) {

}

void DSIpPacket4::setupParameter(uint8_t tos, uint16_t identifier, uint8_t flags,
                                 uint16_t frag, const QVector<uint8_t> &option,
                                 const QVector<uint8_t> &payload) {

    ready = false;
    this->tos = tos;
    this->identifier = identifier;
    this->flags = flags;
    this->frag = frag;
    this->option = option;
    this->payload = payload;

    updateParameter();
    ready = true;

}

void DSIpPacket4::updateParameter() {
    libnet_ptag_t ptag;
    int payloadLength = payload.size();
    int headerLength = LIBNET_IPV4_H + option.size();

    DSPacket::initIpHandle();

    if (!option.empty()) {
        ptag = libnet_build_ipv4_options(option.constData(), option.size(),
                                         handle, 0);
        if (ptag == -1) qDebug() << "raw ip option failed:" << libnet_geterror(handle);
    }
    ptag = libnet_build_ipv4(headerLength + payloadLength, tos,
                identifier, frag, ttl, proto, 0, 
                libnet_name2addr4(handle,
                    const_cast<char *>(source.toStdString().c_str()), LIBNET_DONT_RESOLVE),
                libnet_name2addr4(handle,
                    const_cast<char *>(destination.toStdString().c_str()), LIBNET_DONT_RESOLVE),
                payload.constData(), payload.size(), handle, 0);
    if (ptag == -1) qDebug() << "raw ip packet:" << libnet_geterror(handle);
    qDebug() << "proto: " << proto;

    DSPacket::updatePacketData();
}
