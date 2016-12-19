#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newpacketdialog.h"
#include "ui_newpacketdialog.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    newPacketDlg(new NewPacketDialog(this))
{
    ui->setupUi(this);

    initPacketTable();

    connect(ui->sendAllButton, &QPushButton::clicked, this, &MainWindow::sendAllPackets);
    connect(ui->sendSelectedButton, &QPushButton::clicked, this, &MainWindow::sendSelectedPackets);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete newPacketDlg;
    for (auto c : packets) delete c;
}

void MainWindow::addNewPacket() {
    int newRow = ui->packetTable->rowCount();

    auto set_table_item = [&](const QString &item) {
        static int col = 0;
        ui->packetTable->setItem(newRow, col++, new QTableWidgetItem(item));
        col %= 7;
    };

    ui->packetTable->insertRow(newRow);
    auto newPacket = newPacketDlg->getNewPacket();
    packets.push_back(newPacket);

    set_table_item(QString::number(newRow + 1));
    set_table_item(newPacket->getProto());
    set_table_item(newPacket->getSourceIp());
    set_table_item(newPacket->getDestinationIp());
    set_table_item(newPacket->getCapture());
    set_table_item(QString::number(newPacket->length()));
    QString packetBuf;
    newPacket->dumpPacket(packetBuf);
    set_table_item(packetBuf);

    newPacketDlg->accept();
}

void MainWindow::initPacketTable() {
    ui->packetTable->setColumnWidth(0, 30);
    ui->packetTable->setColumnWidth(1, 60);
    ui->packetTable->setColumnWidth(6, 200);

    connect(ui->newPacketButton, &QPushButton::clicked, this->newPacketDlg, &NewPacketDialog::show);
    connect(newPacketDlg->getUiHandle()->newPacketDlgButtonBox,
            &QDialogButtonBox::accepted, this, &MainWindow::addNewPacket);
    connect(ui->resetButton, &QPushButton::clicked, [=]() {
        for (auto c : packets) delete c;
        packets.clear();
        ui->packetTable->clearContents();
        ui->packetTable->setRowCount(0);
    });
}

void MainWindow::sendAllPackets() {
    for (auto p : packets) p->send();
}

void MainWindow::sendSelectedPackets() {
    auto selected = ui->packetTable->selectionModel()->selectedRows();
    qDebug() << selected.size() << "packets selected";
    for (auto idx : selected) packets[idx.row()]->send();
}
