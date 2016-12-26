#ifndef DSDEVICE_H
#define DSDEVICE_H

#include <QString>
#include <QVector>
#include <QList>
#include <QPair>

/*
 * This class is designed to save
 * all devices and their ip addresses.
 */
class DSDevice {

public:
    using list_node = QPair<int, QString>;  // < ip family, ip address >

    DSDevice() { }
    explicit DSDevice(const QString &devname, int linktype);
    ~DSDevice();

    void pushAddress(int flag, QString ip);
    QString getDeviceName() const;
    QList<QString> getIpAddresses(int flag) const;
    int getLinktype() const; // loopback has a different type with others on BSD system
    int addressCount() const { return iplist.size(); }

private:
    QString name;
    QList<list_node> iplist;
    int linktype;

};


#endif // DSDEVICE_H
