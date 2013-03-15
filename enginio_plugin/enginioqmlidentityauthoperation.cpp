#include "enginioqmlidentityauthoperation.h"
#include "enginioabstractobject.h"
#include "enginioplugin.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlEngine>

/*!
 * \qmltype IdentityAuthOperation
 * \instantiates EnginioQmlIdentityAuthOperation
 * \inqmlmodule enginio-plugin
 * \brief Authentication operation based on username and password.
 *
 * IdentityAuthOperation can be used to login user with username and password,
 * join existing session or log out currently authenticated user.
 */

/*!
 * \qmlproperty Client IdentityAuthOperation::client
 * Enginio client object. This property must be set before the operation is executed.
 */

/*!
 * \qmlproperty Client IdentityAuthOperation::loggedInUser
 * Logged in user or \c undefined if no user has been logged in.
 */

/*!
 * \qmlsignal IdentityAuthOperation::finished()
 *
 * Emitted when the operation completes execution.
 */

/*!
 * \qmlsignal IdentityAuthOperation::error(Error error)
 *
 * Emitted when an error occurs while the operation is being executed. \a error contains
 * the error details.
 */

/*!
 * \qmlmethod void IdentityAuthOperation::execute()
 * Execute operation asynchronously. When the operation finishes, \c finished signal
 * is emitted. If there's an error, both \c error and \c finished signals are
 * emitted.
 */

/*!
 * \qmlmethod void IdentityAuthOperation::cancel()
 * Cancel ongoing operation. \c error signal will be emitted with
 * the \c networkError, \c QNetworkReply::OperationCanceledError.
 */

/*!
 * \qmlmethod void IdentityAuthOperation::loginWithUsernameAndPassword(string username, string password)
 * Set operation to login with the \a username and \a password. If login is
 * successful, \c Client::sessionAuthenticated signal will be emitted.
 * This signal is emitted before the operation's \c finished signal.
 */

/*!
 * \qmlmethod void IdentityAuthOperation::attachToSessionWithToken(string sessionToken)
 * Set operation to attach client to an existing session identified by
 * \a sessionToken. If attaching is successful and session is still active,
 * \c Client::sessionAuthenticated signal will be emitted. This signal
 * is emitted before the operation's \c finished signal.
 */

/*!
 * \qmlmethod void IdentityAuthOperation::logout()
 * Set operation to terminate authenticated session (i.e. logout). If logout is
 * successful, \c Client::sessionTerminated signal will be emitted. This
 * signal is emitted before the operation's \c finished signal.
 */


EnginioQmlIdentityAuthOperation::EnginioQmlIdentityAuthOperation(
        EnginioQmlClient *client, QObject *parent) :
    EnginioIdentityAuthOperation(client, parent)
{
}

EnginioQmlClient * EnginioQmlIdentityAuthOperation::getClient() const
{
    return static_cast<EnginioQmlClient*>(client());
}

QJSValue EnginioQmlIdentityAuthOperation::loggedInUserAsJSValue() const
{
    const EnginioAbstractObject *obj = loggedInUser();
    if (obj) {
        return g_qmlEngine->toScriptValue<QJsonObject>(
                    QJsonDocument::fromJson(obj->toEnginioJson()).object());
    }
    return QJSValue();
}
