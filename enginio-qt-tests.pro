TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    enginio-qt \
    client_offline_tests \
    query_operation_tests \
    object_operation_tests \
    authentication_tests \
    acl_operation_tests \
    file_tests \
    usergroup_tests

# Make sure destination path exists
unix {
    !system( mkdir -p $${OUT_PWD}/bin ) : \
        error( "Unable to create bin directory for tests." )
}
