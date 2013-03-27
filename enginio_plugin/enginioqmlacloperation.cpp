#include "enginioqmlacloperation.h"

#include <QJsonDocument>
#include <QPair>
#include <QSharedPointer>

/*!
 * \qmltype AclOperation
 * \instantiates EnginioQmlAclOperation
 * \inqmlmodule enginio-plugin
 * \brief Operation for fetching and modifying permissions of Enginio objects.
 *
 * Usage:
 * \list
 *     \li Define AclOperation object or create one dynamically using
 *         \l Client::createAclOperation() method.
 *     \li Define which object's permissions should be read/updated by setting
 *         the \l object property.
 *     \li To read object permissions: call execute().
 *     \li To grant new permissions to object: define \l permissionsToGrant
 *         property and call execute().
 *     \li To withdraw permissions from object: define \l permissionsToWithdraw
 *         property and call execute().
 *     \li To set all object permissions: define \l permissionsToSet property
 *         and call execute(). Please note that this will overwrite
 *         all existing permissions on object.
 *     \li After operation's \l finished signal is emitted, object's
 *         read/modified permissions from \l results property.
 * \endlist
 *
 * In order to read or modify object's properties user must be authenticated to
 * Enginio service and have "admin" permission on object.
 *
 * Please note that one operation can be used to grant OR withdraw OR set
 * permissions. You should not define more than one of the properties
 * \l permissionsToGrant, \l permissionsToWithdraw and \l permissionsToSet.
 */

/*!
 * \qmlproperty Client AclOperation::client
 * Enginio client object. This property must be set before the operation is
 * executed.
 */

/*!
 * \qmlproperty object AclOperation::object
 * The Enginio object whose permissions operation reads or changes. Object must
 * have valid \c id and \c objectType properties. All other properties will be
 * ignored.
 */

/*!
 * \qmlproperty object AclOperation::permissionsToSet
 * Permissions that should replace object's current permissions. Permissions
 * should be given as object with array properties for permission names ("read",
 * "update", etc). Each array should contain subjects as objects with "id" and
 * "objectType" properties. For
 * example:
 *
 * \code
 * {
 *     "read": [ { "id": "*", "objectType": "aclSubject" } ],
 *     "update": [ { "id": "*", "objectType": "aclSubject" } ],
 *     "delete": [ { "id": "*", "objectType": "aclSubject" } ],
 *     "admin": [ { "id": " 51517c043379130fc400a31f ", "objectType": "users" } ]
 * }
 * \endcode
 */

/*!
 * \qmlproperty object AclOperation::permissionsToGrant
 * Permissions that should be added to object's current permissions. Permissions
 * should be given as object with array properties for permission names ("read",
 * "update", etc). Each array should contain subjects as objects with "id" and
 * "objectType" properties. For example:
 *
 * \code
 * {
 *     "update": [ { "id": " 51517c043379130fc400a31f ", "objectType": "users" } ]
 * }
 * \endcode
 */

/*!
 * \qmlproperty object AclOperation::permissionsToWithdraw
 * Permissions that should be removed from object's current permissions.
 * Permissions should be given as object with array properties for permission
 * names ("read", "update", etc). Each array should contain subjects as objects
 * with "id" and "objectType" properties. For example:
 *
 * \code
 * {
 *     "update": [ { "id": "*", "objectType": "aclSubject" } ],
 *     "delete": [ { "id": "*", "objectType": "aclSubject" } ]
 * }
 * \endcode */

/*!
 * \qmlproperty Acl AclOperation::results
 * Object permissions after operation have been executed.
 */

/*!
 * \qmlsignal AclOperation::finished()
 *
 * Emitted when the operation completes execution.
 */

/*!
 * \qmlsignal AclOperation::error(Error error)
 *
 * Emitted when an error occurs while the operation is being executed. \a error contains
 * the error details.
 */

/*!
 * \qmlmethod void AclOperation::execute()
 * Execute operation asynchronously. When the operation finishes, \c finished signal
 * is emitted. If there's an error, both \c error and \c finished signals are
 * emitted.
 */

/*!
 * \qmlmethod void AclOperation::cancel()
 * Cancel ongoing operation. \c error signal will be emitted with
 * the \c networkError, \c QNetworkReply::OperationCanceledError.
 */

EnginioQmlAclOperation::EnginioQmlAclOperation(EnginioQmlClient *client,
                                               QObject *parent) :
    EnginioAclOperation(client, parent)
{
}

EnginioQmlClient * EnginioQmlAclOperation::getClient() const
{
    return static_cast<EnginioQmlClient*>(client());
}

QJsonObject EnginioQmlAclOperation::jsonObject() const
{
    return EnginioAcl::objectPairToJson(object());
}

void EnginioQmlAclOperation::setJsonObject(QJsonObject object)
{
    setObject(EnginioAcl::objectJsonToPair(object));
}

EnginioAcl * EnginioQmlAclOperation::results()
{
    QSharedPointer<EnginioAcl> acl = resultAcl();
    return acl.data();
}

QJsonObject EnginioQmlAclOperation::permissionsToSet()
{
    QSharedPointer<EnginioAcl> acl = requestAcl();
    QJsonDocument json = QJsonDocument::fromJson(acl->toJson());
    return json.object();
}

void EnginioQmlAclOperation::setPermissionsToSet(const QJsonObject &permissions)
{
    QSharedPointer<EnginioAcl> acl(new EnginioAcl(permissions));
    setPermissions(acl);
    acl.clear();
}

QJsonObject EnginioQmlAclOperation::permissionsToGrant()
{
    QSharedPointer<EnginioAcl> acl = requestAcl();
    QJsonDocument json = QJsonDocument::fromJson(acl->toJson());
    return json.object();
}

void EnginioQmlAclOperation::setPermissionsToGrant(const QJsonObject &permissions)
{
    QSharedPointer<EnginioAcl> acl(new EnginioAcl(permissions));
    setAddPermissions(acl);
    acl.clear();
}

QJsonObject EnginioQmlAclOperation::permissionsToWithdraw()
{
    QSharedPointer<EnginioAcl> acl = requestAcl();
    QJsonDocument json = QJsonDocument::fromJson(acl->toJson());
    return json.object();
}

void EnginioQmlAclOperation::setPermissionsToWithdraw(const QJsonObject &permissions)
{
    QSharedPointer<EnginioAcl> acl(new EnginioAcl(permissions));
    setDeletePermissions(acl);
    acl.clear();
}

