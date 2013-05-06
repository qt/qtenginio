include($$PWD/../enginio-qt/enginio.pri)
QT       += network testlib
QT       -= gui

TARGET = tst_filetest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += ../common/common.h

SOURCES += \
    tst_filetest.cpp \
    ../common/common.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

RESOURCES += \
    testfiles.qrc
