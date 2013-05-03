/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://qt.digia.com/contact-us
**
** This file is part of the Enginio Qt Client Library.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
****************************************************************************/

#include "enginioidentityauthoperation_p.h"
#include "enginioabstractobject.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

/*!
 * \class EnginioIdentityAuthOperation
 * \inmodule enginio-client
 * \brief Authentication operation based on username and password.
 *
 * IdentityAuthOperation can be used to login user with username and password,
 * join existing session or log out currently authenticated user.
 */

EnginioIdentityAuthOperationPrivate::EnginioIdentityAuthOperationPrivate(
        EnginioIdentityAuthOperation *op) :
    EnginioOperationPrivate(op),
    m_type(Enginio::NullAuthOperation),
    m_username(),
    m_password(),
    m_sessionToken(),
    m_loggedInUser(0),
    m_loggedInUserGroups()

{
}

EnginioIdentityAuthOperationPrivate::~EnginioIdentityAuthOperationPrivate()
{
    if (m_loggedInUser)
        delete m_loggedInUser;

    while (!m_loggedInUserGroups.isEmpty()) {
        delete m_loggedInUserGroups.takeFirst();
    }
}

QString EnginioIdentityAuthOperationPrivate::requestPath() const
{
    switch (m_type) {
    case Enginio::IdentityLoginAuthOperation:
        return QStringLiteral("/v1/auth/identity");
    case Enginio::AttachAuthOperation:
    case Enginio::LogoutAuthOperation:
        return QStringLiteral("/v1/session");
    default:
        return QString();
    }
}

QNetworkReply * EnginioIdentityAuthOperationPrivate::doRequest(
        const QUrl &backendUrl)
{
    Q_Q(EnginioIdentityAuthOperation);

    QString path = requestPath();
    QString error;

    if (m_type == Enginio::NullAuthOperation)
        error = QStringLiteral("Unknown operation type");
    else if (path.isEmpty())
        error = QStringLiteral("Request URL creation failed");

    if (!error.isEmpty()) {
        setError(EnginioError::RequestError, error);
        emit q->finished();
        return 0;
    }

    QUrl url(backendUrl);
    url.setPath(path);
    url.setQuery(urlQuery());

    QNetworkRequest req = enginioRequest(url);

    if (!m_sessionToken.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Using session token" << m_sessionToken;
        req.setRawHeader("Enginio-Backend-Session", m_sessionToken.toLatin1());
    }

    QNetworkAccessManager *netManager = m_client->networkManager();

    switch (m_type) {
    case Enginio::IdentityLoginAuthOperation:
        req.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));
        return netManager->post(req, requestData());
    case Enginio::AttachAuthOperation:
        return netManager->get(req);
    case Enginio::LogoutAuthOperation:
        return netManager->deleteResource(req);
    default:
        return 0;
    }
}

void EnginioIdentityAuthOperationPrivate::handleResults()
{
    if (m_type == Enginio::LogoutAuthOperation) {
        if (m_loggedInUser) {
            delete m_loggedInUser;
            m_loggedInUser = 0;
        }

        while (!m_loggedInUserGroups.isEmpty()) {
            delete m_loggedInUserGroups.takeFirst();
        }

        m_client->setSessionToken(QString());
        return;
    }

    QByteArray data = m_reply->readAll();

    qDebug() << "=== Reply" << q_ptr << m_reply->operation() << m_reply->url();
    qDebug() << data;
    qDebug() << "=== Reply end ===";

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        setError(EnginioError::RequestError, QStringLiteral("Invalid reply data"));
        return;
    }

    QJsonObject json = doc.object();
    if (!json.contains(QStringLiteral("sessionToken"))) {
        setError(EnginioError::RequestError, QStringLiteral("Invalid reply data"));
        return;
    }

    QString token = json.value(QStringLiteral("sessionToken")).toString();

    QJsonObject userJson = json.value(QStringLiteral("user")).toObject();
    if (!userJson.isEmpty()) {
        if (m_loggedInUser)
            delete m_loggedInUser;
        m_loggedInUser = m_client->createObject(QStringLiteral("users"));
        m_loggedInUser->fromEnginioJson(userJson);
    }

    QJsonArray groupsJson = json.value(QStringLiteral("usergroups")).toArray();
    for (int i = 0; i < groupsJson.size(); i++) {
        QJsonObject groupJson = groupsJson.at(i).toObject();
        QString objectType = groupJson.value(QStringLiteral("objectType")).toString();
        if (objectType == QStringLiteral("usergroups")) {
            EnginioAbstractObject *group = m_client->createObject(objectType);
            group->fromEnginioJson(groupJson);
            m_loggedInUserGroups.append(group);
        }
    }

    if (!token.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Got session token" << token;
        m_client->setSessionToken(token);
    }
}

QByteArray EnginioIdentityAuthOperationPrivate::requestData() const
{
    QByteArray data;
    if (m_type == Enginio::IdentityLoginAuthOperation) {
        data += "{\"username\":\"";
        data += m_username.toUtf8();
        data += "\",\"password\":\"";
        data += m_password.toUtf8();
        data += "\"}";
    }
    return data;
}

/*!
 * Create a new identity authorization operation. \a client must be
 * a valid pointer to the EnginioClient object. \a parent is optional.
 */
EnginioIdentityAuthOperation::EnginioIdentityAuthOperation(
        EnginioClient *client,
        QObject *parent) :
    EnginioOperation(client,
                     *new EnginioIdentityAuthOperationPrivate(this),
                     parent)
{
    qDebug() << this << "created";
}

/*!
 * Constructor used in inheriting classes.
 */
EnginioIdentityAuthOperation::EnginioIdentityAuthOperation(
        EnginioClient *client,
        EnginioIdentityAuthOperationPrivate &dd,
        QObject *parent) :
    EnginioOperation(client, dd, parent)
{
    qDebug() << this << "created";
}

/*!
 * Destructor.
 */
EnginioIdentityAuthOperation::~EnginioIdentityAuthOperation()
{
    qDebug() << this << "deleted";
}

/*!
 * Set operation to login with the \a username and \a password. If login is
 * successful, \c EnginioClient::sessionAuthenticated signal will be emitted.
 * This signal is emitted before the operation's \c finished signal.
 */
void EnginioIdentityAuthOperation::loginWithUsernameAndPassword(
        const QString &username,
        const QString &password)
{
    qDebug() << "Login:" << username;

    Q_D(EnginioIdentityAuthOperation);
    d->m_type = Enginio::IdentityLoginAuthOperation;
    d->m_username = username;
    d->m_password = password;
}

/*!
 * Set operation to attach client to an existing session identified by
 * \a sessionToken. If attaching is successful and session is still active,
 * \c EnginioClient::sessionAuthenticated signal will be emitted. This signal
 * is emitted before the operation's \c finished signal.
 */
void EnginioIdentityAuthOperation::attachToSessionWithToken(
        const QString &sessionToken)
{
    qDebug() << "Attach to session:" << sessionToken;

    Q_D(EnginioIdentityAuthOperation);
    d->m_type = Enginio::AttachAuthOperation;
    d->m_sessionToken = sessionToken;
}

/*!
 * Set operation to terminate authenticated session (i.e. logout). If logout is
 * successful, \c EnginioClient::sessionTerminated signal will be emitted. This
 * signal is emitted before the operation's \c finished signal.
 */
void EnginioIdentityAuthOperation::logout()
{
    qDebug() << "Logout";

    Q_D(EnginioIdentityAuthOperation);
    d->m_type = Enginio::LogoutAuthOperation;
}

/*!
 * Returns currently logged in user or 0 if no user has been logged in.
 * Note that object will be deleted when the operation is deleted or if the
 * operation is executed again.
 */
const EnginioAbstractObject * EnginioIdentityAuthOperation::loggedInUser() const
{
    Q_D(const EnginioIdentityAuthOperation);
    return d->m_loggedInUser;
}

/*!
 * Returns all usergroups that the currently logged in user belongs to.
 * Returned list is empty if there's no logged in user or user doesn't belong
 * to any groups. Note that objects in the list will be deleted when the operation
 * is deleted or executed again.
 */
QList<EnginioAbstractObject*> EnginioIdentityAuthOperation::loggedInUserGroups()
{
    Q_D(EnginioIdentityAuthOperation);
    return d->m_loggedInUserGroups;
}
