QT       += testlib enginio
QT       -= gui

TARGET = tst_authenticationtest
CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_authenticationtest.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h
