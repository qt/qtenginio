QT       += testlib enginio
QT       -= gui

TARGET = tst_clientofflinetest
CONFIG   += console testcase
CONFIG   -= app_bundle

DESTDIR += ../../../bin

TEMPLATE = app

SOURCES += tst_clientofflinetest.cpp
