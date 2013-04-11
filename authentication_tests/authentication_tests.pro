QT       += network testlib
QT       -= gui

TARGET = tst_authenticationtest
CONFIG   += console
CONFIG   -= app_bundle

unix: DESTDIR = ../bin

TEMPLATE = app

SOURCES += \
    tst_authenticationtest.cpp \
    ../common/common.cpp

HEADERS += ../common/common.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../enginio-qt/enginio_client/release/ -lenginioclient
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../enginio-qt/enginio_client/debug/ -lenginioclientd
else:unix: LIBS += -L$$OUT_PWD/../enginio-qt/enginio_client/ -lenginioclient

macx {
    QMAKE_POST_LINK += install_name_tool -change libenginioclient.1.dylib @executable_path/../enginio-qt/enginio_client/libenginioclient.1.0.0.dylib $(TARGET)
}


INCLUDEPATH += $$PWD/../enginio-qt/enginio_client
DEPENDPATH += $$PWD/../enginio-qt/enginio_client
