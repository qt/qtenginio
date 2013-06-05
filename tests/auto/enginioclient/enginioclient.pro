QT       += testlib enginio
QT       -= gui

DEFINES += TEST_FILE_PATH=\\\"$$_PRO_FILE_PWD_/../common/enginio.png\\\"

TARGET = tst_enginioclient
CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_enginioclient.cpp \

HEADERS += ../common/common.h
