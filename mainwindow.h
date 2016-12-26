#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include "newpacketdialog.h"
#include "DSPacket/DSPacket.h"
#include "DSDevice/DSDevice.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    NewPacketDialog *newPacketDlg;
    QVector<DSPacket *> packets;
    QVector<DSDevice> devicelist;

    void sendSelectedPackets() const;
    void sendAllPackets() const;
    void addNewPacket();
    void initPacketTable();
    void initDeviceList();
    void initSignal();
    void reloadPackets();       // reload the packets to the table view
    void popUpMessageBox(const QString &title, const QString &message) const;
    void exportToPcapFile();

    static void *get_in_addr(struct sockaddr *sa);
};

#endif // MAINWINDOW_H
