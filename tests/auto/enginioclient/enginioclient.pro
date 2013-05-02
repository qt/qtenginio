QT       += testlib enginio
QT       -= gui

TARGET = tst_enginioclient
CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_enginioclient.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h
