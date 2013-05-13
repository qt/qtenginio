TEMPLATE = app

DEFINES += ENGINIO_SAMPLE_NAME=\\\"todo\\\"

QT += quick qml enginio
SOURCES += ../main.cpp

mac: CONFIG -= app_bundle

OTHER_FILES += ../config.js todo.qml
RESOURCES += ../qml.qrc
