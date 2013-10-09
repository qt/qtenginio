TEMPLATE = app

DEFINES += ENGINIO_SAMPLE_NAME=\\\"users\\\"

include(../../common/backendhelper/backendhelper.pri)

QT += quick qml enginio widgets qml
SOURCES += ../main.cpp

mac: CONFIG -= app_bundle

OTHER_FILES += users.qml Browse.qml Login.qml Register.qml
RESOURCES += users.qrc