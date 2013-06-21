TEMPLATE = subdirs

SUBDIRS += \
    enginioclient \
    enginiomodel \
    files \

qtHaveModule(quick) {
    SUBDIRS += qmltests
}
