requires(qtHaveModule(network))

TEMPLATE = subdirs

QMAKE_DOCS = $$PWD/../doc/qtenginio.qdocconf
load(qt_docs)

SUBDIRS += enginio_client

qtHaveModule(quick) {
    SUBDIRS += enginio_plugin
    enginio_plugin.depends = enginio_client
}
