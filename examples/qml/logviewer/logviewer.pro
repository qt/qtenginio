TEMPLATE = app

DEFINES += ENGINIO_SAMPLE_NAME=\\\"logviewer\\\"

QT += quick qml enginio
SOURCES += ../main.cpp

mac: CONFIG -= app_bundle

OTHER_FILES += ../config.js logviewer.qml
RESOURCES += ../qml.qrc logviewer.qrc
