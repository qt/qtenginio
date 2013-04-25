QT       += testlib enginio
QT       -= gui

TARGET = tst_queryoperationtest
CONFIG   += console testcase
CONFIG   -= app_bundle

DESTDIR += ../../../bin

TEMPLATE = app

SOURCES += \
    tst_queryoperationtest.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h
