#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include "newpacketdialog.h"
#include "DSPacket/DSPacket.h"

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

    void sendSelectedPackets() const;
    void sendAllPackets() const;
    void addNewPacket();
    void initPacketTable();
    void initSignals();
    void reloadPackets();
    void popUpMassageBox(const QString &title, const QString &message) const;
    void exportToPcapFile();

};

#endif // MAINWINDOW_H
