QT       += network testlib
QT       -= gui

TARGET = tst_acloperationtest
CONFIG   += console
CONFIG   -= app_bundle

unix: DESTDIR = ../bin

TEMPLATE = app

SOURCES += \
    tst_acloperationtest.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../enginio-qt/enginio_client/release/ -lenginioclient
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../enginio-qt/enginio_client/debug/ -lenginioclientd
else:unix: LIBS += -L$$OUT_PWD/../enginio-qt/enginio_client/ -lenginioclient

INCLUDEPATH += $$PWD/../enginio-qt/enginio_client
DEPENDPATH += $$PWD/../enginio-qt/enginio_client
