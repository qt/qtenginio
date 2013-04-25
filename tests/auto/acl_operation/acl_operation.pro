QT       += testlib enginio
QT       -= gui

TARGET = tst_acloperationtest
CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_acloperationtest.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h
