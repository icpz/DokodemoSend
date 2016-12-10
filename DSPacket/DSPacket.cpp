#include "DSPacket.h"
#include <unistd.h>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iomanip>
#include <QDebug>

int DSPacket::send() const {
    usleep(delay);
    return ready ? libnet_write(handle) : 0;
}

int DSPacket::length() const {
    return packet.length();
}

void DSPacket::dumpPacket(QString &buf) const {
    std::ostringstream oss;
    buf.clear();
    for (auto c : packet)
        oss << std::hex << std::uppercase
            << std::setw(2) << std::setfill('0')
            << (uint32_t)c << " ";
    buf = oss.str().c_str();
}

void DSPacket::updateIpHeader(int protoSize) {
    libnet_ptag_t ptag = 0;

    if (family == AF_INET) {
        qDebug() << "building ipv4...";
        ptag = libnet_build_ipv4(LIBNET_IPV4_H + protoSize,
                0, 242, 0, ttl, proto, 0,
                libnet_name2addr4(handle,
                    const_cast<char *>(source.toStdString().c_str()), LIBNET_DONT_RESOLVE),
                libnet_name2addr4(handle,
                    const_cast<char *>(destination.toStdString().c_str()), LIBNET_DONT_RESOLVE),
                nullptr, 0, handle, 0);
    }
    if (ptag == -1) qDebug() << "build tcp failed: " << libnet_geterror(handle);
}

void DSPacket::initHandle() {
    char errbuf[LIBNET_ERRBUF_SIZE];
    
    if (handle) libnet_destroy(handle);

    handle = libnet_init(family == AF_INET ? LIBNET_RAW4 : LIBNET_RAW6,
                device.toStdString().c_str(), errbuf);

    if (handle == nullptr) qDebug() << "libnet init failed: " << errbuf;
}

void DSPacket::updatePacketData() {

    packet.clear();
    uint8_t *packetBuf = nullptr;
    uint32_t packetSize;

    libnet_pblock_coalesce(handle, &packetBuf, &packetSize);
    if (packetBuf == nullptr) qDebug() << "get pblock failed";
    std::copy_n(packetBuf, packetSize, std::back_inserter(packet));
    free(packetBuf - handle->aligner);
}
