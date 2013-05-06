include($$PWD/../enginio-qt/enginio.pri)

QT       += network testlib
QT       -= gui

TARGET = tst_acloperationtest
CONFIG   += console
CONFIG   -= app_bundle

unix: DESTDIR = ../bin

TEMPLATE = app

SOURCES += \
    tst_acloperationtest.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
