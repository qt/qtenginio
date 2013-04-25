TARGET = Enginio
QT       += network
DEFINES += ENGINIOCLIENT_LIBRARY

load(qt_module)

###FIXME make lib compile with QT_NO_CAST_FROM_ASCII
DEFINES -= QT_NO_CAST_TO_ASCII
DEFINES -= QT_NO_CAST_FROM_ASCII
DEFINES -= QT_NO_CAST_FROM_BYTEARRAY
DEFINES -= QT_NO_CAST_TO_BYTEARRAY

SOURCES += \
    enginioclient.cpp \
    enginiojsonobject.cpp \
    enginiooperation.cpp \
    enginioobjectoperation.cpp \
    enginiojsonobjectfactory.cpp \
    enginioerror.cpp \
    enginioqueryoperation.cpp \
    enginioobjectmodel.cpp \
    enginioabstractobject.cpp \
    enginiojsonwriter.cpp \
    enginioidentityauthoperation.cpp \
    enginioacl.cpp \
    enginioacloperation.cpp \
    enginiofileoperation.cpp

HEADERS += \
    enginioclient.h\
    enginioclient_global.h \
    enginioclient_p.h \
    enginioabstractobject.h \
    enginiojsonobject.h \
    enginiooperation.h \
    enginiooperation_p.h \
    enginioobjectoperation.h \
    enginioobjectoperation_p.h \
    enginioabstractobjectfactory.h \
    enginiojsonobjectfactory.h \
    enginioerror.h \
    enginioerror_p.h \
    enginioqueryoperation.h \
    enginioqueryoperation_p.h \
    enginioobjectmodel.h \
    enginioobjectmodel_p.h \
    enginiojsonwriter_p.h \
    enginioidentityauthoperation.h \
    enginioidentityauthoperation_p.h \
    enginioacl.h \
    enginioacl_p.h \
    enginioacloperation.h \
    enginioacloperation_p.h \
    enginiofileoperation.h \
    enginiofileoperation_p.h
