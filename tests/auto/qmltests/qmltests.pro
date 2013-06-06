TEMPLATE = app

SOURCES += tst_qmltest.cpp

TARGET = tst_qmltests

QT += enginio
CONFIG += testcase

QT += qmltest

DEFINES += QUICK_TEST_SOURCE_DIR=\\\"$$_PRO_FILE_PWD_\\\"

OTHER_FILES += \
    tst_enginioclient.qml
