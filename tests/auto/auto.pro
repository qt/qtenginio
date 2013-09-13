TEMPLATE = subdirs

SUBDIRS += \
    enginioclient \
    files \
    notifications \

qtHaveModule(quick) {
    SUBDIRS += qmltests
}

qtHaveModule(widgets) {
    SUBDIRS += enginiomodel
}
