TARGETPATH = Enginio

CONFIG += force_independent

load(qml_plugin)
include($$PWD/../../enginio.pri)

TARGET = enginioplugin
TARGET.module_name = Enginio

QT += qml quick enginio enginio-private

SOURCES += \
    enginioqmlclient.cpp \
    enginioqmlmodel.cpp \
    enginioplugin.cpp \
    enginioqmlobjectmodel.cpp \
    enginioqmlqueryoperation.cpp \
    enginioqmlobjectoperation.cpp \
    enginioqmlidentityauthoperation.cpp \
    enginioqmlacloperation.cpp \
    enginioqmlfileoperation.cpp

HEADERS += \
    enginioqmlclient.h \
    enginioqmlmodel.h \
    enginioplugin.h \
    enginioqmlobjectmodel.h \
    enginioqmlqueryoperation.h \
    enginioqmlobjectoperation.h \
    enginioqmlidentityauthoperation.h \
    enginioqmlacloperation.h \
    enginioqmlfileoperation.h \
    enginioqmloblejctadaptor_p.h

QMLDIRFILE = $${_PRO_FILE_PWD_}/qmldir

CONFIG += no_cxx_module

lessThan(QT_MINOR_VERSION, 1) {
    # CONFIG += force_independent does not work properly prior Qt 5.1.
    # Make sure we do not overwrite the release plugin with the debug.
    DESTDIR = ../../qml/$${TARGET.module_name}
    TARGET = $$TARGET$$qtPlatformTargetSuffix()
}

copy2build.input = QMLDIRFILE
copy2build.output = ../../qml/$${TARGET.module_name}/qmldir
!contains(TEMPLATE_PREFIX, vc):copy2build.variable_out = PRE_TARGETDEPS
copy2build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
copy2build.name = COPY ${QMAKE_FILE_IN}
copy2build.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += copy2build
