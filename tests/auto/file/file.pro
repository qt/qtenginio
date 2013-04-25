QT       += testlib enginio
QT       -= gui

TARGET = tst_filetest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DESTDIR = ../../bin

HEADERS += ../common/common.h

SOURCES += \
    tst_filetest.cpp \
    ../common/common.cpp

RESOURCES += \
    testfiles.qrc
