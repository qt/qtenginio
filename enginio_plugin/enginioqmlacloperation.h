#ifndef ENGINIOQMLACLOPERATION_H
#define ENGINIOQMLACLOPERATION_H

#include "enginioacloperation.h"
#include "enginioqmlclient.h"

#include <QJsonArray>
#include <QJsonObject>

class EnginioQmlAclOperation : public EnginioAclOperation
{
    Q_OBJECT
    Q_DISABLE_COPY(EnginioQmlAclOperation)
    Q_PROPERTY(EnginioQmlClient* client READ getClient WRITE setClient)
    Q_PROPERTY(QJsonObject object READ jsonObject WRITE setJsonObject)
    Q_PROPERTY(QJsonObject permissionsToSet READ permissionsToSet WRITE setPermissionsToSet)
    Q_PROPERTY(QJsonObject permissionsToGrant READ permissionsToGrant WRITE setPermissionsToGrant)
    Q_PROPERTY(QJsonObject permissionsToWithdraw READ permissionsToWithdraw WRITE setPermissionsToWithdraw)
    Q_PROPERTY(EnginioAcl* results READ results)

public:
    EnginioQmlAclOperation(EnginioQmlClient *client = 0,
                           QObject *parent = 0);

    EnginioQmlClient * getClient() const;
    QJsonObject jsonObject() const;
    void setJsonObject(QJsonObject object);
    EnginioAcl * results();
    QJsonObject permissionsToSet();
    void setPermissionsToSet(const QJsonObject &permissions);
    QJsonObject permissionsToGrant();
    void setPermissionsToGrant(const QJsonObject &permissions);
    QJsonObject permissionsToWithdraw();
    void setPermissionsToWithdraw(const QJsonObject &permissions);
};

#endif // ENGINIOQMLACLOPERATION_H
