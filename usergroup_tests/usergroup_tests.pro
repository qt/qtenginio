include($$PWD/../enginio-qt/enginio.pri)

QT       += testlib
QT       -= gui

TARGET = tst_usergrouptest
CONFIG   += console
CONFIG   -= app_bundle

unix: DESTDIR = ../bin

TEMPLATE = app

SOURCES += \
    tst_usergrouptest.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
