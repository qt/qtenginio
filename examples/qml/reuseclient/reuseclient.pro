TEMPLATE = app

DEFINES += ENGINIO_SAMPLE_NAME=\\\"reuseclient\\\"

QT += quick qml enginio
SOURCES += ../main.cpp

mac: CONFIG -= app_bundle

OTHER_FILES += ../config.js reuseclient.qml
RESOURCES += ../qml.qrc
