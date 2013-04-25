QT       += testlib enginio
QT       -= gui

TARGET = tst_acloperationtest
CONFIG   += console
CONFIG   -= app_bundle

DESTDIR = ../../bin

TEMPLATE = app

SOURCES += \
    tst_acloperationtest.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h
