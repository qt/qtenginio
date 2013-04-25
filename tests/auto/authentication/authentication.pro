QT       += testlib enginio
QT       -= gui

TARGET = tst_authenticationtest
CONFIG   += console
CONFIG   -= app_bundle

DESTDIR = ../../bin

TEMPLATE = app

SOURCES += \
    tst_authenticationtest.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h
