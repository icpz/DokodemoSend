#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newpacketdialog.h"
#include "ui_newpacketdialog.h"
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    newPacketDlg(new NewPacketDialog(this))
{
    ui->setupUi(this);

    initPacketTable();
    initSignals();

}

MainWindow::~MainWindow()
{
    delete ui;
    delete newPacketDlg;
    for (auto c : packets) delete c;
}

void MainWindow::addNewPacket() {
    auto newPacket = newPacketDlg->getNewPacket();
    if (!newPacket) return;

    packets.push_back(newPacket);
    reloadPackets();
    newPacketDlg->accept();
}

void MainWindow::initPacketTable() {
    ui->packetTable->setColumnWidth(0, 30);
    ui->packetTable->setColumnWidth(1, 60);
    ui->packetTable->setColumnWidth(6, 200);
}

void MainWindow::initSignals() {
    connect(ui->sendAllButton, &QPushButton::clicked, this, &MainWindow::sendAllPackets);
    connect(ui->sendSelectedButton, &QPushButton::clicked, this, &MainWindow::sendSelectedPackets);

    connect(ui->newPacketButton, &QPushButton::clicked, this->newPacketDlg, &NewPacketDialog::show);
    connect(newPacketDlg->getUiHandle()->newPacketDlgButtonBox,
            &QDialogButtonBox::accepted, this, &MainWindow::addNewPacket);

    connect(ui->resetButton, &QPushButton::clicked, [=]() {
        for (auto c : packets) delete c;
        packets.clear();
        reloadPackets();
    });
    connect(ui->delPacketButton, &QPushButton::clicked, [=]() {
        auto selected = ui->packetTable->selectionModel()->selectedRows();
        qDebug() << selected.size() << "packets will be deleted";
        for (const auto &idx : selected) {
            delete packets[idx.row()];
            packets[idx.row()] = nullptr;
        }
        packets.erase(std::remove(std::begin(packets), std::end(packets), nullptr), std::end(packets));
        reloadPackets();
    });
}

void MainWindow::sendAllPackets() const {
    for (auto p : packets) p->send();
    popUpMassageBox(tr("Packet Sent"), QString::number(packets.size()) + tr(" packets sent"));
}

void MainWindow::sendSelectedPackets() const {
    auto selected = ui->packetTable->selectionModel()->selectedRows();
    qDebug() << selected.size() << "packets selected";
    for (auto idx : selected) packets[idx.row()]->send();
    popUpMassageBox(tr("Packet Sent"), QString::number(selected.size()) + tr(" packets sent"));
}

void MainWindow::reloadPackets() {
    ui->packetTable->clearContents();
    ui->packetTable->setRowCount(0);

    auto set_table_item = [&](int row, const QString &item) {
        static int col = 0;
        ui->packetTable->setItem(row, col++, new QTableWidgetItem(item));
        col %= 7;
    };

    for (int i = 0; i < packets.size(); ++i) {
        ui->packetTable->insertRow(i);
        auto p = packets[i];

        set_table_item(i, QString::number(i + 1));
        set_table_item(i, p->getProto());
        set_table_item(i, p->getSourceIp());
        set_table_item(i, p->getDestinationIp());
        set_table_item(i, p->getCapture());
        set_table_item(i, QString::number(p->length()));
        QString packetBuf;
        p->dumpPacket(packetBuf);
        set_table_item(i, packetBuf);
    }
}

void MainWindow::popUpMassageBox(const QString &title, const QString &message) const {
    QMessageBox msgBox(QMessageBox::Information, title,
                       message, QMessageBox::Ok, nullptr);
    msgBox.exec();
}
