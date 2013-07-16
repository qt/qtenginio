QT       += testlib enginio enginio-private
QT       -= gui

TARGET = tst_notifications
CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_notifications.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h
