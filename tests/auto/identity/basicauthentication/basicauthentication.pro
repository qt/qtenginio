QT       += testlib enginio
QT       -= gui

TARGET = tst_basicauthentication
CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

include(../common/common.pri)

SOURCES += tst_basicauthentication.cpp