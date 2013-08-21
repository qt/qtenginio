requires(qtHaveModule(quick))

TARGETPATH = Enginio

QT += qml quick enginio enginio-private

load(qml_plugin)

TARGET = enginioplugin
TARGET.module_name = Enginio

SOURCES += \
    enginioqmlclient.cpp \
    enginioqmlmodel.cpp \
    enginioplugin.cpp \
    enginioqmlreply.cpp \

HEADERS += \
    enginioqmlclient.h \
    enginioqmlmodel.h \
    enginioplugin.h \
    enginioqmlobjectadaptor_p.h \
    enginioqmlclient_p.h \
    enginioqmlreply.h \

QMLDIRFILE = $${_PRO_FILE_PWD_}/qmldir
