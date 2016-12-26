
#include "DSDevice.h"
#include <algorithm>
#include <iterator>

DSDevice::DSDevice(const QString &devname, int linktype)
    : name(devname), linktype(linktype) {
}

DSDevice::~DSDevice() {

}

void DSDevice::pushAddress(int flag, QString ip) {
    iplist.push_back(qMakePair(flag, ip));
}

QString DSDevice::getDeviceName() const {
    return name;
}

QList<QString> DSDevice::getIpAddresses(int flag) const {
    QList<QString> result;
    for (const auto &ip : iplist) {
        if (ip.first == flag) result.push_front(ip.second);
    }
    return result;
}

int DSDevice::getLinktype() const {
    return linktype;
}
