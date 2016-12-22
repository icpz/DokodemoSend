#-------------------------------------------------
#
# Project created by QtCreator 2016-12-05T20:01:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DokodemoSend
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    newpacketdialog.cpp \
    DSDevice/dsdevice.cpp \
    DSPacket/DSPacket.cpp \
    DSPacket/DSTcpPacket.cpp \
    DSPacket/DSUdpPacket.cpp \
    DSPacket/DSIpPacket.cpp \
    DSPacket/DSArpPacket.cpp \
    DSPacket/DSIcmpPacket.cpp \
    get_packet_value.cpp \
    check_packet.cpp \
    generate_packet.cpp

HEADERS  += mainwindow.h \
    newpacketdialog.h \
    DSDevice/dsdevice.h \
    DSPacket/DSPacket.h

FORMS    += mainwindow.ui \
    newpacketdialog.ui

CONFIG += c++14 no_lflags_merge

LIBS += -lpcap -L/usr/local/lib -lnet

INCLUDEPATH += /usr/local/include
