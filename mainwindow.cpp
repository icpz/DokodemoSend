#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newpacketdialog.h"
#include "ui_newpacketdialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <pcap/pcap.h>
#include <fstream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    devicelist(0)
{
    ui->setupUi(this);

    initPacketTable();
    initDeviceList();

    newPacketDlg = new NewPacketDialog(devicelist, this);
    initSignal();

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

void MainWindow::initSignal() {
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
    connect(ui->actionSaveAsPcap, &QAction::triggered, this, &MainWindow::exportToPcapFile);
}

void MainWindow::initDeviceList() {
    pcap_if_t *alldev;
    char errbuf[PCAP_ERRBUF_SIZE];
    char ipBuf[INET6_ADDRSTRLEN];

    if (pcap_findalldevs(&alldev, errbuf)) {
        qDebug() << errbuf << endl;
        QApplication::quit();
    }

    for (auto device = alldev; device; device = device->next) {
        DSDevice dev(tr(device->name), device->flags);
        for (auto addr = device->addresses; addr; addr = addr->next) {
            auto ipFamily = addr->addr->sa_family;
            if(ipFamily == AF_INET || ipFamily == AF_INET6) {
                inet_ntop(ipFamily, get_in_addr((struct sockaddr *)addr->addr), ipBuf, sizeof ipBuf);
                dev.pushAddress(ipFamily, ipBuf);
            }
        }
        if (dev.addressCount()) {
            devicelist.push_back(dev);
        }
    }

    pcap_freealldevs(alldev);
}

void MainWindow::sendAllPackets() const {
    for (auto p : packets) p->send();
    popUpMessageBox(tr("Packet Sent"), QString::number(packets.size()) + tr(" packets sent"));
}

void MainWindow::sendSelectedPackets() const {
    auto selected = ui->packetTable->selectionModel()->selectedRows();
    qDebug() << selected.size() << "packets selected";
    for (auto idx : selected) packets[idx.row()]->send();
    popUpMessageBox(tr("Packet Sent"), QString::number(selected.size()) + tr(" packets sent"));
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

void MainWindow::popUpMessageBox(const QString &title, const QString &message) const {
    QMessageBox msgBox(QMessageBox::Information, title,
                       message, QMessageBox::Ok, nullptr);
    msgBox.exec();
}

void MainWindow::exportToPcapFile() {
    if (packets.size() == 0) {
        popUpMessageBox(tr("Export to file"), tr("There is no packet to export"));
        return;
    }

    // users are expected to select an existing directory here
    QString dirPrefix = QFileDialog::getExistingDirectory(this, tr("Export to directory"));
    if (dirPrefix.size() == 0) return;
    struct pcap_file_header pcap_hdr;
    struct pcap_pkthdr pkt_hdr;
    std::fill_n(reinterpret_cast<uint8_t *>(&pcap_hdr), sizeof pcap_hdr, 0);

    // one device per file
    for (const auto c : devicelist) {
        pcap_hdr.linktype = DLT_RAW;
        pcap_hdr.magic = 0xa1b2c3d4;
        pcap_hdr.version_major = PCAP_VERSION_MAJOR;
        pcap_hdr.version_minor = PCAP_VERSION_MINOR;
        pcap_hdr.sigfigs = 0;
        pcap_hdr.snaplen = 0x40000;
        pcap_hdr.thiszone = 0;

        QString filepath = dirPrefix + "/" + c.getDeviceName() + ".pcap";
        std::ofstream ofs(filepath.toStdString(), std::ios::binary | std::ios::out);
        ofs.write(reinterpret_cast<char *>(&pcap_hdr), sizeof pcap_hdr);

        for (const auto p : packets) {
            if (p->getCapture() != c.getDeviceName()) continue;
            if (p->getProto() == "ARP") continue;   // ARP packets have the link-layer data generated
                                                    // So we skip the ARP packets due to the linktype DLT_RAW
            QVector<uint8_t> packetBuf(p->getPacket());

            gettimeofday(&pkt_hdr.ts, nullptr);
            pkt_hdr.caplen = pkt_hdr.len = packetBuf.size();
            ofs.write(reinterpret_cast<char *>(&pkt_hdr.ts.tv_sec), 4);
            ofs.write(reinterpret_cast<char *>(&pkt_hdr.ts.tv_usec), 4);
            ofs.write(reinterpret_cast<char *>(&pkt_hdr.caplen), 8);
            ofs.write(reinterpret_cast<const char *>(packetBuf.constData()), packetBuf.size());
        }
        ofs.close();
    }
    popUpMessageBox(tr("Export to file"), tr("Packets saved"));
}

void *MainWindow::get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
