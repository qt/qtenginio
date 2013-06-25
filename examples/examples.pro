TEMPLATE = subdirs

qtHaveModule(quick) {
    SUBDIRS += qml
}
qtHaveModule(widgets) {
    SUBDIRS += qt
}
