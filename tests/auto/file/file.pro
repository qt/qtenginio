QT       += testlib enginio
QT       -= gui

TARGET = tst_filetest
CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += ../common/common.h

SOURCES += \
    tst_filetest.cpp \
    ../common/common.cpp

RESOURCES += \
    testfiles.qrc
