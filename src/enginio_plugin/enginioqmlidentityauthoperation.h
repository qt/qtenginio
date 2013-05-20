#ifndef ENGINIOQMLIDENTITYAUTHOPERATION_H
#define ENGINIOQMLIDENTITYAUTHOPERATION_H

#include "enginioidentityauthoperation.h"
#include "enginioqmlclient.h"

#include <QJSValue>

class EnginioQmlIdentityAuthOperation : public EnginioIdentityAuthOperation
{
    Q_OBJECT
    Q_DISABLE_COPY(EnginioQmlIdentityAuthOperation)
    Q_PROPERTY(EnginioQmlClient* client READ getClient WRITE setClient)
    Q_PROPERTY(QJSValue loggedInUser READ loggedInUserAsJSValue)
    Q_PROPERTY(QJSValue loggedInUserGroups READ loggedInUserGroupsAsArray)

public:
    EnginioQmlIdentityAuthOperation(EnginioQmlClient *client = 0,
                                    QObject *parent = 0);

    EnginioQmlClient * getClient() const;
    QJSValue loggedInUserAsJSValue() const;
    QJSValue loggedInUserGroupsAsArray() const;
};

#endif // ENGINIOQMLIDENTITYAUTHOPERATION_H
