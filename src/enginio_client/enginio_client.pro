TARGET = Enginio
QT += network
DEFINES += ENGINIOCLIENT_LIBRARY
MODULE = enginio

!sharedlib {
    win32: CONFIG -= build_all
    else {
        CONFIG -= debug_and_release
        CONFIG += staticlib
    }
}


load(qt_module)

SOURCES += \
    enginioclient.cpp \
    enginioreply.cpp \
    enginiomodel.cpp \
    enginioidentity.cpp \
    enginiofakereply.cpp

HEADERS += \
    chunkdevice_p.h \
    enginioclient.h\
    enginioclient_global.h \
    enginioclient_p.h \
    enginioreply.h \
    enginiomodel.h \
    enginioidentity.h \
    enginioobjectadaptor_p.h \
    enginioreply_p.h \
    enginiofakereply_p.h

