TEMPLATE = subdirs

SUBDIRS += \
    enginioclient \
    notifications \
    identity \

qtHaveModule(gui) {
    SUBDIRS += files
}

qtHaveModule(quick) {
    SUBDIRS += qmltests
}

qtHaveModule(widgets) {
    SUBDIRS += enginiomodel
}
