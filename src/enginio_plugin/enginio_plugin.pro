TARGET = enginioplugin
TARGETPATH = Enginio
TARGET.module_name = Enginio
QT += qml quick enginio

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

QMLDIRFILE = $${_PRO_FILE_PWD_}/qmldir

INCLUDEPATH += $$PWD/../enginio_client
DEPENDPATH += $$PWD/../enginio_client

CONFIG += no_cxx_module
load(qml_plugin)
