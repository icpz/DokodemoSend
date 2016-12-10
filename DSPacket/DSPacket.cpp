#include "DSPacket.h"
#include <algorithm>
#include <iterator>
#include <QDebug>

int DSPacket::send() const {
    return ready ? libnet_write(handle) : 0;
}

int DSPacket::length() const {
    return packet.length();
}

void DSPacket::dumpPacket(QString &buf) const {
    buf.clear();
    for (auto c : packet)
        buf.append(QString::number(c, 16).toUpper() + " ");
}

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
    char errbuf[LIBNET_ERRBUF_SIZE];
    libnet_ptag_t ptag;
    int tcpPacketLength = LIBNET_TCP_H + option.size() + payload.size();

    if (handle) libnet_destroy(handle);

    qDebug() << "Updating Packet...";
    handle = libnet_init(family == AF_INET ? LIBNET_RAW4 : LIBNET_RAW6,
                device.toStdString().c_str(), errbuf);

    if (handle == nullptr) qDebug() << "libnet init failed: " << errbuf;

    if (!option.empty()) {
        ptag = libnet_build_tcp_options(option.constData(), option.size(),
                handle, 0);
    }

    ptag = libnet_build_tcp(srcPort, dstPort, seq, ack, flag, windowsize, 0,
            urgpoint, tcpPacketLength,
            payload.constData(), payload.size(), handle, 0);

    if (ptag == -1) qDebug() << "build tcp failed: " << libnet_geterror(handle);

    if (family == AF_INET) {
        qDebug() << "building ipv4...";
        ptag = libnet_build_ipv4(LIBNET_IPV4_H + tcpPacketLength,
                0, 242, 0, ttl, proto, 0,
                libnet_name2addr4(handle,
                    const_cast<char *>(source.toStdString().c_str()), LIBNET_DONT_RESOLVE),
                libnet_name2addr4(handle,
                    const_cast<char *>(destination.toStdString().c_str()), LIBNET_DONT_RESOLVE),
                nullptr, 0, handle, 0);
    }

    if (ptag == -1) qDebug() << "build ip failed: " << libnet_geterror(handle);

    packet.clear();
    uint8_t *packetBuf = nullptr;
    uint32_t packetSize;
/*
    libnet_pblock_coalesce(handle, &packetBuf, &packetSize);
    if (packetBuf == nullptr) qDebug() << "get pblock failed";
    std::copy_n(packetBuf, packetSize, std::back_inserter(packet));
    free(packetBuf - handle->aligner);
    */
}

