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
}

MainWindow::~MainWindow()
{
    delete ui;
    delete newPacketDlg;
}

void MainWindow::addNewPacket() {
    int newRow = ui->packetTable->rowCount();
    auto dlgUi = newPacketDlg->getUiHandle();

    ui->packetTable->insertRow(newRow);
    ui->packetTable->setItem(newRow, 0, new QTableWidgetItem(tr(newPacketDlg->getNewPacket().c_str())));
    ui->packetTable->setItem(newRow, 1, new QTableWidgetItem(tr("hello")));
    qDebug() << dlgUi->captureComboBox->currentText() << " selected.";
    newPacketDlg->accept();
}

void MainWindow::initPacketTable() {
    ui->packetTable->setColumnWidth(0, 30);

    connect(ui->newPacketButton, &QPushButton::clicked, this->newPacketDlg, &NewPacketDialog::show);
    connect(newPacketDlg->getUiHandle()->newPacketDlgButtonBox,
            &QDialogButtonBox::accepted, this, &MainWindow::addNewPacket);
    connect(ui->resetButton, &QPushButton::clicked, [=]() {
        ui->packetTable->clearContents();
        ui->packetTable->setRowCount(0);
    });
}

