#ifndef NEWPACKETDIALOG_H
#define NEWPACKETDIALOG_H

#include <QDialog>
#include <string>
#include "ui_newpacketdialog.h"
#include <sys/socket.h>
#include "DSDevice/dsdevice.h"
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

    void initSignal();
    void initValidator();
    void initDeviceList();
    int getDeviceByName(const QString &name) const;
    void updateSourceIpCompleter(int family, const QString &dev);
    bool checkCommon() const;
    void popUpErrorMessageBox(const QString &errorMessage) const;

    int getTTL() const { return ui->ttlEdit->text().toInt(); }
    int getDelay() const { return ui->delayEdit->text().toInt(); }
    int getIpFamily() const;
    QString getDevice() const { return ui->captureComboBox->currentText(); }
    QString getSourceIp() const { return ui->srcIPEdit->text(); }
    QString getDestinationIp() const { return ui->dstIPEdit->text(); }
    QVector<uint8_t> getPayload() const { return parse_hex(ui->packetPayload->toPlainText()); }

    DSPacket *generateTcpPacket() const;
    bool checkTcp() const;
    int getTcpFlag() const;

    DSPacket *generateUdpPacket() const;
    bool checkUdp() const;

    DSPacket *generateIpPacket() const;
    bool checkIp() const;
    int getIpFlag() const;

    DSPacket *generateArpPacket() const;
    bool checkArp() const;
    int getArpType() const;

    DSPacket *generateIcmpPacket() const;
    bool checkIcmp() const;

    static void *get_in_addr(struct sockaddr *sa);
    static bool is_ip_valid(int family, const QString &ip);
    static QVector<uint8_t> parse_hex(const QString &hexes);
};

#endif // NEWPACKETDIALOG_H
