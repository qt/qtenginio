QT       += testlib enginio
QT       -= gui

TARGET = tst_objectoperationtest
CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_objectoperationtest.cpp \
    testobjectfactory.cpp \
    selflinkedobject.cpp \
    ../common/common.cpp

HEADERS += \
    testobjectfactory.h \
    selflinkedobject.h \
    ../common/common.h
