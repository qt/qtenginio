QT       += testlib enginio
QT       -= gui

DEFINES += TEST_FILE_PATH=\\\"$$_PRO_FILE_PWD_/../file/enginio.png\\\"

TARGET = tst_enginiomodel
CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_enginiomodel.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h
