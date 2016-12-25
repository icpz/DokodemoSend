#include "newpacketdialog.h"
#include <QString>
#include <arpa/inet.h>

static bool __check_ip_address(int family, const QString &addr) {
    uint8_t ipBuf[16];

    if (inet_pton(family, addr.toStdString().c_str(), ipBuf) == 1) return true;
    return false;
}

static bool __check_range(uint64_t number, uint8_t bitwidth) {
    return number < (1ULL << bitwidth);
}

bool NewPacketDialog::checkTcp() const {

    if (!__check_range(ui->tcpSrcPort->text().toInt(), 16)) {
        popUpErrorMessageBox(tr("Invalid Source Port"));
        goto __bad_case;
    }
    if (!__check_range(ui->tcpDstPort->text().toInt(), 16)) {
        popUpErrorMessageBox(tr("Invalid Destination Port"));
        goto __bad_case;
    }
    if (!__check_range(ui->tcpSeqNumber->text().toInt(), 32)) {
        popUpErrorMessageBox(tr("Invalid Sequence Number"));
        goto __bad_case;
    }
    if (!__check_range(ui->tcpAckNumber->text().toInt(), 32)) {
        popUpErrorMessageBox(tr("Invalid Acknowledgment Number"));
        goto __bad_case;
    }
    if (!__check_range(ui->tcpWindow->text().toInt(), 16)) {
        popUpErrorMessageBox(tr("Invalid Window Size"));
        goto __bad_case;
    }
    if (!__check_range(ui->tcpUrgent->text().toInt(), 16)) {
        popUpErrorMessageBox(tr("Invalid Urgent Pointer"));
        goto __bad_case;
    }
    if (ui->tcpOptions->text().size() % 2 == 1) {
        popUpErrorMessageBox(tr("Even-length TCP Option Expected"));
        goto __bad_case;
    }

    return checkCommon();

__bad_case:
    return false;
}

bool NewPacketDialog::checkUdp() const {

    if (!__check_range(ui->udpSrcPort->text().toInt(), 16)) {
        popUpErrorMessageBox(tr("Invalid Source Port"));
        goto __bad_case;
    }
    if (!__check_range(ui->udpDstPort->text().toInt(), 16)) {
        popUpErrorMessageBox(tr("Invalid Destination Port"));
        goto __bad_case;
    }
    return checkCommon();

__bad_case:
    return false;
}

bool NewPacketDialog::checkIp() const {

    if (!__check_range(ui->ipTos->text().toInt(nullptr, 16), 8)) {
        popUpErrorMessageBox(tr("Invalid TOS"));
        goto __bad_case;
    }
    if (!__check_range(ui->ipIdentifier->text().toInt(), 16)) {
        popUpErrorMessageBox(tr("Invalid Identifier"));
        goto __bad_case;
    }
    if (!__check_range(ui->ipFrag->text().toInt(), 13)) {
        popUpErrorMessageBox(tr("Invalid Fragment Offset"));
        goto __bad_case;
    }
    if (!__check_range(ui->ipProto->text().toInt(), 8)) {
        popUpErrorMessageBox(tr("Invalid Protocol"));
        goto __bad_case;
    }
    if (ui->ipOptions->text().size() % 2 == 1) {
        popUpErrorMessageBox(tr("Even-length IP Option Expected"));
        goto __bad_case;
    }
    return checkCommon();

__bad_case:
    return false;
}

bool NewPacketDialog::checkArp() const {

    if (ui->srcMACEdit->text().size() != 17) {
        popUpErrorMessageBox(tr("Invalid Source MAC Address"));
        goto __bad_case;
    }
    if (ui->dstMACEdit->text().size() != 17) {
        popUpErrorMessageBox(tr("Invalid Destination MAC Address"));
        goto __bad_case;
    }
    return checkCommon();

__bad_case:
    return false;
}

bool NewPacketDialog::checkIcmp() const {

    if (!__check_range(ui->icmpCodeEdit->text().toInt(), 8)) {
        popUpErrorMessageBox(tr("Invalid Code"));
        goto __bad_case;
    }
    if (!__check_range(ui->icmpTypeEdit->text().toInt(), 8)) {
        popUpErrorMessageBox(tr("Invalid Type"));
        goto __bad_case;
    }
    if (ui->icmpHexMode->isChecked()) {
        if (!__check_range(ui->icmpHexEdit->text().toLong(nullptr, 16), 32)) {
            popUpErrorMessageBox(tr("Invalid Rest of ICMP"));
            goto __bad_case;
        }
    } else if (ui->icmpIDAndSeqMode->isChecked()) {
        if (!__check_range(ui->icmpIDEdit->text().toInt(), 16)) {
            popUpErrorMessageBox(tr("Invalid ID"));
            goto __bad_case;
        }
        if (!__check_range(ui->icmpSeqEdit->text().toInt(), 16)) {
            popUpErrorMessageBox(tr("Invalid Sequence"));
            goto __bad_case;
        }
    }
    return checkCommon();

__bad_case:
    return false;
}

bool NewPacketDialog::checkCommon() const {

    if (ui->captureComboBox->currentIndex() == 0) {
        popUpErrorMessageBox(tr("Capture Must Be Specific"));
        goto __bad_case;
    }
    if (!__check_ip_address(getIpFamily(), getSourceIp())) {
        popUpErrorMessageBox(tr("Invalid Source Ip Address"));
        goto __bad_case;
    }
    if (!__check_ip_address(getIpFamily(), getDestinationIp())) {
        popUpErrorMessageBox(tr("Invalid Destination Ip Address"));
        goto __bad_case;
    }
    if (!__check_range(ui->ttlEdit->text().toInt(), 8)) {
        popUpErrorMessageBox(tr("Invalid TTL"));
        goto __bad_case;
    }
    if (!__check_range(ui->delayEdit->text().toInt(), 16)) {
        popUpErrorMessageBox(tr("Invalid Delay"));
        goto __bad_case;
    }
    if ((ui->packetPayload->toPlainText().size() % 2) == 1) {
        popUpErrorMessageBox(tr("Even-length Payload Expected"));
        goto __bad_case;
    }
    return true;

__bad_case:
    return false;
}
