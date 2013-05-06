include($$PWD/../enginio-qt/enginio.pri)

QT       += network testlib
QT       -= gui

TARGET = tst_authenticationtest
CONFIG   += console
CONFIG   -= app_bundle

unix: DESTDIR = ../bin

TEMPLATE = app

SOURCES += \
    tst_authenticationtest.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
