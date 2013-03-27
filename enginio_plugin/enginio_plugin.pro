TEMPLATE = lib
TARGET = enginioplugin
QT += qml quick
CONFIG += qt plugin

uri = io.engin
DESTDIR = io/engin

# Input
SOURCES += \
    enginioqmlclient.cpp \
    enginioplugin.cpp \
    enginioqmlobjectmodel.cpp \
    enginioqmlqueryoperation.cpp \
    enginioqmlobjectoperation.cpp \
    enginioqmlidentityauthoperation.cpp \
    enginioqmlacloperation.cpp

HEADERS += \
    enginioqmlclient.h \
    enginioplugin.h \
    enginioqmlobjectmodel.h \
    enginioqmlqueryoperation.h \
    enginioqmlobjectoperation.h \
    enginioqmlidentityauthoperation.h \
    enginioqmlacloperation.h

OTHER_FILES = qmldir

URIDIR = $$replace(uri, \\., /)

!equals($$_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/$$URIDIR/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir

installPath = $$[QT_INSTALL_IMPORTS]/$$replace(uri, \\., /)
qmldir.path = $$installPath
target.path = $$installPath
INSTALLS += target qmldir


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../enginio_client/release/ -lenginioclient
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../enginio_client/debug/ -lenginioclient
else:unix: LIBS += -L$$OUT_PWD/../enginio_client/ -lenginioclient

INCLUDEPATH += $$PWD/../enginio_client
DEPENDPATH += $$PWD/../enginio_client
