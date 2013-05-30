defineReplace(targetSubDir) {
    CONFIG(debug, debug|release): return(debug)
    else: return(release)
}

# Only use force_independent for non-developer build
!contains(QT_CONFIG, private_tests): CONFIG += force_independent

!sharedlib {
    win32: CONFIG -= build_all
    else: CONFIG -= debug_and_release
}

!qtHaveModule(enginio) {
    # The module is not installed, try finding the pri file.
    module_pri = mkspecs/modules/qt_lib_enginio.pri
    exists($$PWD/$$module_pri): include($$PWD/$$module_pri) # in-source build
    else: exists($$OUT_PWD/../enginio-qt/$$module_pri): include($$OUT_PWD/../enginio-qt/$$module_pri) # shadow build
    else: exists($$[QT_INSTALL_PREFIX]/$$module_pri): include($$[QT_INSTALL_PREFIX]/$$module_pri) # should be a developer build
    else: error("Enginio client library module pri not found.")
}

QT += core network enginio

!sharedlib: win32 {
    COPY_DLL_CMD = \"$${QT.enginio.libs}/$${QT.enginio.name}$$qtPlatformTargetSuffix().dll\" \"$$OUT_PWD/$$targetSubDir()\"
    COPY_DLL_CMD = $$replace(COPY_DLL_CMD, /, \\)
    QMAKE_POST_LINK += $${QMAKE_COPY} $${COPY_DLL_CMD} &
}
