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

#include "enginioclient_p.h"
#include "enginioreply.h"
#include "enginiomodel.h"
#include "enginioidentity.h"

#include <QNetworkReply>
#include <QSslError>
#include <QtCore/qthreadstorage.h>

/*!
  \module enginio-client
  \title Enginio Client Interface

  This interface provides access to the Enginio service through a set of C++ classes
  based on Qt.
*/

/*!
  \class EnginioClient
  \inmodule enginio-qt
  \ingroup enginio-client
  \brief EnginioClient handles API keys, sessions and authorization.
  \mainclass

  When using Enginio you need to set up your backend using this class.

  The \l backendId and \l backendSecret are required for Engino to work.
  Once the client is set up you can use it to make queries to the backend
  or use higher level API such as \l {EnginioModelCpp}{EnginioModel}.
*/

/*!
  \fn EnginioClient::sessionAuthenticated() const
  \brief Emitted when a user logs in.
*/

/*!
  \fn EnginioClient::sessionTerminated() const
  \brief Emitted when a user logs out.
*/

/*!
    \enum EnginioClient::Operation

    This enum describes which operation a query uses.

    \value ObjectOperation Operate on objects
    \value ObjectAclOperation Operate on the ACL
    \value UserOperation Operate on users
    \value UsergroupOperation Operate on groups
    \value UsergroupMembersOperation Operate on group members
*/

const QString EnginioString::pageSize = QStringLiteral("pageSize");
const QString EnginioString::limit = QStringLiteral("limit");
const QString EnginioString::offset = QStringLiteral("offset");
const QString EnginioString::include = QStringLiteral("include");
const QString EnginioString::query = QStringLiteral("query");
const QString EnginioString::message = QStringLiteral("message");
const QString EnginioString::results = QStringLiteral("results");
const QString EnginioString::_synced = QStringLiteral("_synced");
const QString EnginioString::objectType = QStringLiteral("objectType");
const QString EnginioString::id = QStringLiteral("id");
const QString EnginioString::username = QStringLiteral("username");
const QString EnginioString::password = QStringLiteral("password");
const QString EnginioString::sessionToken = QStringLiteral("sessionToken");
const QString EnginioString::authIdentity = QStringLiteral("auth/identity");
const QString EnginioString::files = QStringLiteral("files");
const QString EnginioString::search = QStringLiteral("search");
const QString EnginioString::session = QStringLiteral("session");
const QString EnginioString::users = QStringLiteral("users");
const QString EnginioString::usergroups = QStringLiteral("usergroups");
const QString EnginioString::object = QStringLiteral("object");
const QString EnginioString::url = QStringLiteral("url");
const QString EnginioString::access = QStringLiteral("access");
const QString EnginioString::sort = QStringLiteral("sort");
const QString EnginioString::count = QStringLiteral("count");
const QString EnginioString::targetFileProperty = QStringLiteral("targetFileProperty");
const QString EnginioString::members = QStringLiteral("members");
const QString EnginioString::propertyName = QStringLiteral("propertyName");
const QString EnginioString::apiEnginIo = QStringLiteral("https://api.engin.io");

EnginioClientPrivate::EnginioClientPrivate(EnginioClient *client) :
    q_ptr(client),
    _identity(),
    m_apiUrl(EnginioString::apiEnginIo),
    m_networkManager()
{
    assignNetworkManager();

    _request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/json"));
    qRegisterMetaType<EnginioClient*>();
    qRegisterMetaType<EnginioModel*>();
    qRegisterMetaType<EnginioReply*>();
    qRegisterMetaType<EnginioIdentity*>();
    qRegisterMetaType<EnginioAuthentication*>();
}

EnginioClientPrivate::~EnginioClientPrivate()
{
    foreach (const QMetaObject::Connection &identityConnection, _identityConnections)
        QObject::disconnect(identityConnection);
    QObject::disconnect(_networkManagerConnection);
}

/*!
 * \brief Create a new EnginioClient.
 * \param parent the QObject parent.
 *
 * \sa backendId, backendSecret
 */
EnginioClient::EnginioClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new EnginioClientPrivate(this))
{
}

/*!
 * \internal
 */
EnginioClient::EnginioClient(QObject *parent, EnginioClientPrivate *d)
    : QObject(parent)
    , d_ptr(d)
{
}

/*!
 * Destroys the EnginioClient.
 *
 * This ends the Enginio session.
 */
EnginioClient::~EnginioClient()
{}

/*!
 * \property EnginioClient::backendId
 * \brief The unique ID for the used Enginio backend.
 *
 * The backend ID determines which Enginio backend is used
 * by this instance of EnginioClient. The backend ID and \l backendSecret are
 * required for Enginio to work.
 * It is possible to use several Enginio backends simultaneously
 * by having several instances of EnginioClient.
 * \sa backendSecret
 */
QByteArray EnginioClient::backendId() const
{
    Q_D(const EnginioClient);
    return d->m_backendId;
}

void EnginioClient::setBackendId(const QByteArray &backendId)
{
    Q_D(EnginioClient);
    if (d->m_backendId != backendId) {
        d->m_backendId = backendId;
        d->_request.setRawHeader("Enginio-Backend-Id", d->m_backendId);
        emit backendIdChanged(backendId);
    }
}

/*!
 * \property EnginioClient::backendSecret
 * \brief The backend secret that corresponds to the \l backendId.
 * The secret is used to authenticate the Enginio connection.
 */
QByteArray EnginioClient::backendSecret() const
{
    Q_D(const EnginioClient);
    return d->m_backendSecret;
}

void EnginioClient::setBackendSecret(const QByteArray &backendSecret)
{
    Q_D(EnginioClient);
    if (d->m_backendSecret != backendSecret) {
        d->m_backendSecret = backendSecret;
        d->_request.setRawHeader("Enginio-Backend-Secret", d->m_backendSecret);
        emit backendSecretChanged(backendSecret);
    }
}

/*!
 * \property EnginioClient::apiUrl
 * \brief Enginio backend URL.
 *
 * The API URL determines the server used by Enginio.
 * Usually it is not needed to change the default URL.
 */
QUrl EnginioClient::apiUrl() const
{
    Q_D(const EnginioClient);
    return d->m_apiUrl;
}

void EnginioClient::setApiUrl(const QUrl &apiUrl)
{
    Q_D(EnginioClient);
    if (d->m_apiUrl != apiUrl) {
        d->m_apiUrl = apiUrl;
        d->ignoreSslErrorsIfNeeded();
        emit apiUrlChanged(apiUrl);
    }
}

/*!
 * \brief Get the QNetworkAccessManager used by the Enginio library.
 *
 * Note that it will be deleted with the client object.
 */
QNetworkAccessManager * EnginioClient::networkManager()
{
    Q_D(EnginioClient);
    return d->networkManager();
}

void EnginioClient::ignoreSslErrors(QNetworkReply* reply,
                                    const QList<QSslError> &errors)
{
    QList<QSslError>::ConstIterator i = errors.constBegin();
    while (i != errors.constEnd()) {
        qWarning() << "Ignoring SSL error:" << i->errorString();
        i++;
    }
    reply->ignoreSslErrors(errors);
}

/*!
 * \brief Search.
 *
 * \return EnginioReply containing the status and the result once it is finished.
 * \sa EnginioReply, create(), query(), update(), remove()
 */
EnginioReply *EnginioClient::search(const QJsonObject &query)
{
    Q_D(EnginioClient);

    QNetworkReply *nreply = d->query<QJsonObject>(query, EnginioClientPrivate::SearchOperation);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);
    return ereply;
}

/*!
 * \brief Query the database.
 *
 * \return EnginioReply containing the status and the result once it is finished.
 * \sa EnginioReply, create(), update(), remove()
 */
EnginioReply* EnginioClient::query(const QJsonObject &query, const Operation operation)
{
    Q_D(EnginioClient);

    QNetworkReply *nreply = d->query<QJsonObject>(query, static_cast<EnginioClientPrivate::Operation>(operation));
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);
    return ereply;
}

/*!
 * \brief Insert a new \a object into the database.
 *
 * The \a operation is the area in which the object gets created. It defaults to \l ObjectOperation
 * to create new objects by default.
 * \return EnginioReply containing the status of the query and the data once it is finished.
 * \sa EnginioReply, query(), update(), remove()
 */
EnginioReply* EnginioClient::create(const QJsonObject &object, const Operation operation)
{
    Q_D(EnginioClient);

    if (object.empty())
        return 0;

    QNetworkReply *nreply = d->create<QJsonObject>(object, operation);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);

    return ereply;
}

/*!
 * \brief Update an existing \a object in the database.
 *
 * The \a operation is the area in which the object gets created. It defaults to \l ObjectOperation
 * to create new objects by default.
 * \return EnginioReply containing the status of the query and the data once it is finished.
 * \sa EnginioReply, create(), query(), remove()
 */
EnginioReply* EnginioClient::update(const QJsonObject &object, const Operation operation)
{
    Q_D(EnginioClient);

    if (object.empty())
        return 0;

    QNetworkReply *nreply = d->update<QJsonObject>(object, operation);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);

    return ereply;
}

/*!
 * \brief Remove an existing \a object from the database.
 *
 * The \a operation is the area in which the object gets created. It defaults to \l ObjectOperation
 * to create new objects by default.
 * \return EnginioReply containing the status of the query and the data once it is finished.
 * \sa EnginioReply, create(), query(), update()
 */
EnginioReply* EnginioClient::remove(const QJsonObject &object, const Operation operation)
{
    Q_D(EnginioClient);

    if (object.empty())
        return 0;

    QNetworkReply *nreply = d->remove<QJsonObject>(object, operation);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);

    return ereply;
}

/*!
 * \property EnginioClient::identity
 * Represents a user.
 * \sa EnginioIdentity
 */
EnginioIdentity *EnginioClient::identity() const
{
    Q_D(const EnginioClient);
    return d->identity();
}

void EnginioClient::setIdentity(EnginioIdentity *identity)
{
    Q_D(EnginioClient);
    if (d->_identity == identity)
        return;
    d->setIdentity(identity);
}

/*!
  \brief Stores a \a file attached to an \a object in Enginio

  Each uploaded file needs to be associated with an object in the database.
  \note The upload will only work with the propper server setup: in the dashboard create a property
  of the type that you will use. Set this property to be a reference to files.

  Each uploaded file needs to be associated with an object in the database.

  In order to upload a file, first create an object:
  \snippet enginioclient/tst_enginioclient.cpp upload-create-object

  Then do the actual upload:
  \snippet enginioclient/tst_enginioclient.cpp upload

  Note: There is no need to directly delete files.
  Instead when the object that contains the link to the file gets deleted,
  the file will automatically be deleted as well.

  \sa downloadFile()
*/
EnginioReply* EnginioClient::uploadFile(const QJsonObject &object, const QUrl &file)
{
    Q_D(EnginioClient);

    QNetworkReply *nreply = d->uploadFile<QJsonObject>(object, file);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);

    return ereply;
}

/*!
  \brief Download a file stored in Enginio

  \snippet enginioclient/tst_enginioclient.cpp download
  The propertyName can be anything, but it must be the same as the one used to upload the file with.
  This way one object can have several files attached to itself (one per propertyName).
*/
EnginioReply* EnginioClient::downloadFile(const QJsonObject &object)
{
    Q_D(EnginioClient);

    QNetworkReply *nreply = d->downloadFile<QJsonObject>(object);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);

    return ereply;
}

Q_GLOBAL_STATIC(QThreadStorage<QNetworkAccessManager*>, NetworkManager)

void EnginioClientPrivate::assignNetworkManager()
{
    Q_ASSERT(!m_networkManager);

    m_networkManager = prepareNetworkManagerInThread();
    _networkManagerConnection = QObject::connect(m_networkManager, &QNetworkAccessManager::finished, EnginioClientPrivate::ReplyFinishedFunctor(this));
}

QNetworkAccessManager *EnginioClientPrivate::prepareNetworkManagerInThread()
{
    QNetworkAccessManager *qnam;
    qnam = NetworkManager->localData();
    if (!qnam) {
        qnam = new QNetworkAccessManager(); // it will be deleted by QThreadStorage.
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
        qnam->connectToHostEncrypted(EnginioString::apiEnginIo);
#endif
        NetworkManager->setLocalData(qnam);
    }
    return qnam;
}

QJsonObject EnginioClient::identityToken() const
{
    Q_D(const EnginioClient);
    return d->identityToken();
}
