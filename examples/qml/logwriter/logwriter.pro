TEMPLATE = app

DEFINES += ENGINIO_SAMPLE_NAME=\\\"logwriter\\\"

QT += quick qml enginio
SOURCES += ../main.cpp

mac: CONFIG -= app_bundle

OTHER_FILES += ../config.js logwriter.qml
