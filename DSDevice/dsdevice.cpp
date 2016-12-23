
#include "dsdevice.h"
#include <algorithm>
#include <iterator>

DSDevice::DSDevice(const QString &devname, int linktype)
    : name(devname), linktype(linktype) {
}

DSDevice::~DSDevice() {

}

void DSDevice::push_address(int flag, QString ip) {
    iplist.push_back(qMakePair(flag, ip));
}

QString DSDevice::get_device_name() const {
    return name;
}

QList<QString> DSDevice::get_ip_addresses(int flag) const {
    QList<QString> result;
    for (const auto &ip : iplist) {
        if (ip.first == flag) result.push_front(ip.second);
    }
    return result;
}

int DSDevice::get_linktype() const {
    return linktype;
}
