#ifndef NEWPACKETDIALOG_H
#define NEWPACKETDIALOG_H

#include <QDialog>
#include <string>
#include "ui_newpacketdialog.h"
#include <sys/socket.h>
#include "dsdevice.h"
#include "DSPacket/DSPacket.h"
#include <QVector>
#include <stdint.h>

namespace Ui {
class NewPacketDialog;
}

class NewPacketDialog : public QDialog
{
    Q_OBJECT

public:

    enum {TAB_TCP, TAB_UDP, TAB_IP, TAB_ARP, TAB_ICMP};

    explicit NewPacketDialog(QWidget *parent = 0);
    ~NewPacketDialog();

    Ui::NewPacketDialog *getUiHandle() { return ui; }
    DSPacket *getNewPacket();
    void show();

private:
    Ui::NewPacketDialog *ui;
    QVector<DSDevice> devicelist;

    void initDeviceList();
    void initSignals();
    int getDeviceByName(const QString &name) const;
    void updateSourceIpCompleter(int family, const QString &dev);
    int getIpFamily() const;
    void switchIpTab();

    DSPacket *generateTcpPacket() const;
    bool checkTcp() const;
    int getTcpFlag() const;

    DSPacket *generateUdpPacket() const;
    bool checkUdp() const;

    DSPacket *generateIpPacket() const;
    bool checkIp() const;
    int getIpFlag() const;

    static void *get_in_addr(struct sockaddr *sa);
    static bool is_ip_valid(int family, const QString &ip);
    static QVector<uint8_t> parse_hex(const QString &hexes);
};

#endif // NEWPACKETDIALOG_H
