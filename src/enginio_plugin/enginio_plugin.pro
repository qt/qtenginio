TARGET = enginioplugin
TARGETPATH = io/engin
QT += qml quick

SOURCES += \
    enginioqmlclient.cpp \
    enginioplugin.cpp \
    enginioqmlobjectmodel.cpp \
    enginioqmlqueryoperation.cpp \
    enginioqmlobjectoperation.cpp \
    enginioqmlidentityauthoperation.cpp \
    enginioqmlacloperation.cpp \
    enginioqmlfileoperation.cpp

HEADERS += \
    enginioqmlclient.h \
    enginioplugin.h \
    enginioqmlobjectmodel.h \
    enginioqmlqueryoperation.h \
    enginioqmlobjectoperation.h \
    enginioqmlidentityauthoperation.h \
    enginioqmlacloperation.h \
    enginioqmlfileoperation.h

OTHER_FILES = qmldir

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../enginio_client/release/ -lEnginio
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../enginio_client/debug/ -lEnginiod
else:unix: LIBS += -L$$OUT_PWD/../enginio_client/ -lEnginio

INCLUDEPATH += $$PWD/../enginio_client
DEPENDPATH += $$PWD/../enginio_client

CONFIG += no_cxx_module
load(qml_plugin)
