#ifndef NEWPACKETDIALOG_H
#define NEWPACKETDIALOG_H

#include <QDialog>
#include <string>
#include "ui_newpacketdialog.h"
#include <sys/socket.h>
#include "dsdevice.h"

namespace Ui {
class NewPacketDialog;
}

class NewPacketDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewPacketDialog(QWidget *parent = 0);
    ~NewPacketDialog();

    Ui::NewPacketDialog *getUiHandle() { return ui; }
    std::string getNewPacket();
    void show();

private:
    Ui::NewPacketDialog *ui;
    QVector<DSDevice> devicelist;

    void initDeviceList();
    int getDeviceByName(const QString &name);
    void updateSourceIpCompleter(int family, const QString &dev);

    static void *get_in_addr(struct sockaddr *sa);
    static int get_ip_family(const QString &af);
    static bool is_ip_valid(int family, const QString &ip);
};

#endif // NEWPACKETDIALOG_H
