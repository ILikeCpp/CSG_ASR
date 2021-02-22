#-------------------------------------------------
#
# Project created by QtCreator 2021-02-03T11:07:01
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CSG_ASR
TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += ../include \
               ./ASRService \
               ./Common \
               ./GCSClient

SOURCES += main.cpp\
    GCSClient/GCSClientMgr.cpp \
    GCSClient/HttpClient.cpp \
        MainWindow.cpp \
    ASRService/SinoVoiceASRService.cpp \
    ASRService/ASRServiceAbs.cpp \
    Common/AccountInfo.cpp \
    Common/CommonTool.cpp \
    Common/FileReader.cpp

HEADERS  += MainWindow.h \
    ASRService/SinoVoiceASRService.h \
    ASRService/ASRServiceAbs.h \
    Common/AccountInfo.h \
    Common/applog.h \
    Common/common_define.h \
    Common/CommonTool.h \
    Common/FileReader.h \
    GCSClient/ASRListennerAbs.h \
    GCSClient/GCSClientMgr.h \
    GCSClient/HttpClient.h

msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

win32 {
LIBS += -L$$PWD/../lib/win_x86 \
        -lhci_asr_recorder \
        -lhci_sys
}

linux {

}

DESTDIR = ../bin/win_x86
