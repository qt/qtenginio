QT       += testlib enginio
QT       -= gui

TARGET = tst_usergroup
CONFIG += console testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_usergrouptest.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h

