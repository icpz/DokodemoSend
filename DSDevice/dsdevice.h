#ifndef DSDEVICE_H
#define DSDEVICE_H

#include <QString>
#include <QVector>
#include <QList>
#include <QPair>

class DSDevice {

public:
    using list_node = QPair<int, QString>;

    DSDevice() { }
    explicit DSDevice(const QString &devname, int linktype);
    ~DSDevice();

    void push_address(int flag, QString ip);
    QString get_device_name() const;
    QList<QString> get_ip_addresses(int flag) const;
    int get_linktype() const;
    int address_count() const { return iplist.size(); }

private:
    QString name;
    QList<list_node> iplist;
    int linktype;

};


#endif // DSDEVICE_H
