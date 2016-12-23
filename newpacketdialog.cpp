#include "newpacketdialog.h"
#include "ui_newpacketdialog.h"
#include <pcap.h>
#include <QDebug>
#include <QCompleter>
#include <sys/socket.h>
#include <QMessageBox>

NewPacketDialog::NewPacketDialog(const QVector<DSDevice> devs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewPacketDialog),
    devicelist(devs)
{
    ui->setupUi(this);

    initSignal();
    initValidator();

    QStringList devList;
    for (const auto &c : devicelist) {
        devList << c.get_device_name();
    }
    ui->captureComboBox->addItems(devList);

}

NewPacketDialog::~NewPacketDialog()
{
    delete ui;
}

void NewPacketDialog::initValidator() {
    static const QRegularExpression ipAddressReg("^(\\d{1,3}\\.){3}\\d{1,3}$"),
            decNumber16Reg("^\\d{1,5}$"), decNumber32Reg("^\\d{1,10}$"), decNumber8Reg("^\\d{1,3}$"),
            hexNumber32Reg("^[\\da-fA-F]{1,8}$"), hexNumber8Reg("^[\\da-fA-F]{1,2}$"),
            macAddressReg("^([\\da-fA-F]{2}-){5}[\\da-fA-F]{2}$"),
            hexOptionReg("^([\\da-fA-F]{2}[- ]?)*$");

    auto setValidator = [&](std::initializer_list<QLineEdit *> wlist, const QRegularExpression &reg) {
        for (auto w : wlist) w->setValidator(new QRegularExpressionValidator(reg, this));
    };

    setValidator({ui->srcIPEdit, ui->dstIPEdit}, ipAddressReg);

    setValidator({ui->tcpAckNumber, ui->tcpSeqNumber}, decNumber32Reg);
    setValidator({ui->tcpDstPort, ui->tcpSrcPort, ui->tcpUrgent, ui->tcpWindow,
                  ui->udpDstPort, ui->udpSrcPort,
                  ui->ipIdentifier, ui->ipFrag,
                  ui->icmpIDEdit, ui->icmpSeqEdit},
                 decNumber16Reg);

    setValidator({ui->ttlEdit, ui->ipProto, ui->icmpCodeEdit, ui->icmpTypeEdit}, decNumber8Reg);
    setValidator({ui->ipTos}, hexNumber8Reg);
    setValidator({ui->icmpHexEdit}, hexNumber32Reg);
    setValidator({ui->srcMACEdit, ui->dstMACEdit}, macAddressReg);
    setValidator({ui->tcpOptions, ui->ipOptions}, hexOptionReg);

}

void NewPacketDialog::initSignal() {

    ui->icmpHexEdit->hide();

    connect(ui->captureComboBox,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            [&](const QString &dev) {
                int ipFamily = this->getIpFamily();
                updateSourceIpCompleter(ipFamily, dev);
            });

    connect(ui->icmpIDAndSeqMode, &QRadioButton::clicked,
            [&](bool) {
                ui->icmpLabelID->setText(tr("ID"));
                ui->icmpLabelSEQ->show();
                ui->icmpIDEdit->show();
                ui->icmpSeqEdit->show();
                ui->icmpHexEdit->hide();
            });
    connect(ui->icmpHexMode, &QRadioButton::clicked,
            [&](bool) {
                ui->icmpLabelID->setText(tr("Hex"));
                ui->icmpLabelSEQ->hide();
                ui->icmpSeqEdit->hide();
                ui->icmpIDEdit->hide();
                ui->icmpHexEdit->show();
            });
    connect(ui->packetPayload, &QPlainTextEdit::textChanged,
            [&]() {
                static bool processing;
                if (processing) return;
                auto payloadEdit = ui->packetPayload;
                processing = true;
                std::string textBuf(payloadEdit->toPlainText().toStdString());

                textBuf.erase(
                    std::remove_if(std::begin(textBuf), std::end(textBuf),
                        [](char c) {
                            return !((std::isalpha(c) && std::toupper(c) <= 'F') || std::isdigit(c));
                        }), std::end(textBuf));

                payloadEdit->setPlainText(textBuf.c_str());
                auto cursor = payloadEdit->textCursor();
                cursor.setPosition(textBuf.size());
                payloadEdit->setTextCursor(cursor);
                processing = false;
            });
}

void NewPacketDialog::show() {

    QDialog::show();
}

void NewPacketDialog::updateSourceIpCompleter(int family, const QString &dev) {
    int devIndex = getDeviceByName(dev);
    if (devIndex == -1) return;
    QStringList completerList(devicelist[getDeviceByName(dev)].get_ip_addresses(family));
    auto newCompleter = new QCompleter(completerList, this);

    newCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    ui->srcIPEdit->setCompleter(newCompleter);
}

void NewPacketDialog::popUpErrorMessageBox(const QString &errorMessage) const {
    QMessageBox errMsgBox(QMessageBox::Critical, tr("Parameter Error"),
                          errorMessage, QMessageBox::Ok, nullptr);
    errMsgBox.exec();
}

QVector<uint8_t> NewPacketDialog::parse_hex(const QString &hexes) {
    QVector<uint8_t> result;
    QString filtered;

    std::remove_copy_if(std::cbegin(hexes), std::cend(hexes),
                    std::back_inserter(filtered), [](QChar c) {
                        return c == QChar(' ')
                            || c == QChar('\n')
                            || c == QChar('-');
                    });
    for (int i = 0; i < filtered.size(); i += 2) {
        result.push_back(filtered.mid(i, 2).toUInt(nullptr, 16));
    }
    return result;
}
