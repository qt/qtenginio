QT       += network
QT       -= gui

TARGET = enginioclient
TEMPLATE = lib
VERSION = 0.0.1

# Don't include major version in DLL name
win32: TARGET_EXT = .dll

DEFINES += ENGINIOCLIENT_LIBRARY
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

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
    enginioacloperation.cpp

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
    enginioacloperation_p.h

headers.files = $$HEADERS

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
        headers.path = /opt/usr/include/Enginio
    } else {
        target.path = /usr/lib
        headers.path = /usr/include/Enginio
    }
    INSTALLS += \
        target \
        headers
}
