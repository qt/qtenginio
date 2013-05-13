TEMPLATE = app

DEFINES += ENGINIO_SAMPLE_NAME=\\\"loganalyzer\\\"

QT += quick qml enginio
SOURCES += ../main.cpp

mac: CONFIG -= app_bundle

OTHER_FILES += ../config.js loganalyzer.qml
RESOURCES += ../qml.qrc
