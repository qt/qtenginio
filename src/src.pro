requires(qtHaveModule(quick))
requires(qtHaveModule(network))

TEMPLATE = subdirs

QMAKE_DOCS = $$PWD/../doc/enginio-qt.qdocconf

CONFIG += ordered

SUBDIRS += \
    enginio_client \
    enginio_plugin
