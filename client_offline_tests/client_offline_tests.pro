include($$PWD/../enginio-qt/enginio.pri)

QT       += testlib
QT       -= gui

TARGET = tst_clientofflinetest
CONFIG   += console
CONFIG   -= app_bundle

unix: DESTDIR = ../bin

TEMPLATE = app

SOURCES += tst_clientofflinetest.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
