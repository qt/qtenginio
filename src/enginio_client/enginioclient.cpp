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
  \brief EnginioClient handles all communication with the Enginio server

  The Enginio server supports several separate "backends" with each account.
  By setting the \l backendId and \l backendSecret a backend is chosen.
  After setting the ID and secret interaction with the server is possible.
  The information about the backend is available on the Enginio Dashboard
  after logging in to \l {http://engin.io}{Enginio}.
  \code
    EnginioClient *client = new EnginioClient(parent);
    client->setBackendId(QByteArrayLiteral("YOUR_BACKEND_ID"));
    client->setBackendSecret(QByteArrayLiteral("YOUR_BACKEND_SECRET"));
  \endcode

  The basic functions used to interact with the backend are
  \l create(), \l query(), \l remove() and \l update().
  It is possible to do a fulltext search on the server using \l search().
  For file handling \l downloadFile() and \l uploadFile() are provided.

  \note After the request has finished, it is the responsibility of the
  user to delete the EnginioReply object at an appropriate time.
  Do not directly delete it inside the slot connected to finished().
  You can use the deleteLater() function.

  In order to make queries that return an array of data more convenient
  a model is provided by \l {EnginioModelCpp}{EnginioModel}.
*/

/*!
  \fn void EnginioClient::error(EnginioReply *reply)
  \brief This signal is emitted when a request to the backend returns an error.

  The \a reply contains the details about the error that occured.
  \sa EnginioReply
*/

/*!
  \fn void EnginioClient::finished(EnginioReply *reply)
  \brief This signal is emitted when a request to the backend finishes.

  The \a reply contains the data returned. This signal is emitted for both, successful requests and failed ones.
  \sa EnginioReply
*/

/*!
  \property EnginioClient::authenticationState
  \brief The state of the authentication.

  Enginio provides convenient user management.
  The authentication state reflects whether the current user is authenticated.
  \sa AuthenticationState
*/

/*!
  \fn EnginioClient::sessionAuthenticated(EnginioReply *reply) const
  \brief Emitted when a user logs in.

  The \a reply contains the details about the login.

  \sa sessionAuthenticationError(), EnginioReply
*/

/*!
  \fn EnginioClient::sessionAuthenticationError(EnginioReply *reply) const
  \brief Emitted when a user login fails.

  The \a reply contains the details about why the login failed.
  \sa sessionAuthenticated(), EnginioReply
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
    \value FileOperation Operate with files
    \value UserOperation Operate on users
    \value UsergroupOperation Operate on groups
    \value UsergroupMembersOperation Operate on group members
*/

/*!
    \enum EnginioClient::AuthenticationState

    This enum describes the state of the user authentication.
    \value NotAuthenticated No attempt to authenticate was made
    \value Authenticating Authentication request has been sent to the server
    \value Authenticated Authentication was successful
    \value AuthenticationFailure Authentication failed

    \sa authenticationState
*/

ENGINIOCLIENT_EXPORT bool gEnableEnginioDebugInfo = !qEnvironmentVariableIsSet("ENGINIO_DEBUG_INFO");

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
const QString EnginioString::file = QStringLiteral("file");
const QString EnginioString::fileName = QStringLiteral("fileName");
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
const QString EnginioString::status = QStringLiteral("status");
const QString EnginioString::empty = QStringLiteral("empty");
const QString EnginioString::complete = QStringLiteral("complete");
const QString EnginioString::incomplete = QStringLiteral("incomplete");
const QString EnginioString::headers = QStringLiteral("headers");
const QString EnginioString::payload = QStringLiteral("payload");
const QString EnginioString::variant = QStringLiteral("variant");

EnginioClientPrivate::EnginioClientPrivate(EnginioClient *client) :
    q_ptr(client),
    _identity(),
    _serviceUrl(EnginioString::apiEnginIo),
    _networkManager(),
    _uploadChunkSize(512 * 1024),
    _authenticationState(EnginioClient::NotAuthenticated)
{
    assignNetworkManager();

    _request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/json"));
}

void EnginioClientPrivate::init()
{
    qRegisterMetaType<EnginioClient*>();
    qRegisterMetaType<EnginioModel*>();
    qRegisterMetaType<EnginioReply*>();
    qRegisterMetaType<EnginioIdentity*>();
    qRegisterMetaType<EnginioAuthentication*>();

    QObject::connect(q_ptr, &EnginioClient::sessionTerminated, AuthenticationStateTrackerFunctor(this));
    QObject::connect(q_ptr, &EnginioClient::sessionAuthenticated, AuthenticationStateTrackerFunctor(this, EnginioClient::Authenticated));
    QObject::connect(q_ptr, &EnginioClient::sessionAuthenticationError, AuthenticationStateTrackerFunctor(this, EnginioClient::AuthenticationFailure));
    QObject::connect(q_ptr, &EnginioClient::identityChanged, AuthenticationStateTrackerIdentFunctor(this));
}

EnginioClientPrivate::~EnginioClientPrivate()
{
    foreach (const QMetaObject::Connection &identityConnection, _identityConnections)
        QObject::disconnect(identityConnection);
    foreach (const QMetaObject::Connection &connection, _connections)
        QObject::disconnect(connection);
    QObject::disconnect(_networkManagerConnection);
}

/*!
  \brief Creates a new EnginioClient with \a parent as QObject parent.
*/
EnginioClient::EnginioClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new EnginioClientPrivate(this))
{
    Q_D(EnginioClient);
    d->init();
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
    return d->_backendId;
}

void EnginioClient::setBackendId(const QByteArray &backendId)
{
    Q_D(EnginioClient);
    if (d->_backendId != backendId) {
        d->_backendId = backendId;
        d->_request.setRawHeader("Enginio-Backend-Id", d->_backendId);
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
    return d->_backendSecret;
}

void EnginioClient::setBackendSecret(const QByteArray &backendSecret)
{
    Q_D(EnginioClient);
    if (d->_backendSecret != backendSecret) {
        d->_backendSecret = backendSecret;
        d->_request.setRawHeader("Enginio-Backend-Secret", d->_backendSecret);
        emit backendSecretChanged(backendSecret);
    }
}

/*!
  \property EnginioClient::serviceUrl
  \brief Enginio backend URL.
  \internal

  The API URL determines the server used by Enginio.
  Usually it is not needed to change the default URL.
*/

/*!
  \fn EnginioClient::serviceUrlChanged(const QUrl &url)
  \internal
*/

/*!
  \internal
*/
QUrl EnginioClient::serviceUrl() const
{
    Q_D(const EnginioClient);
    return d->_serviceUrl;
}

/*!
    \internal
*/
void EnginioClient::setServiceUrl(const QUrl &serviceUrl)
{
    Q_D(EnginioClient);
    if (d->_serviceUrl != serviceUrl) {
        d->_serviceUrl = serviceUrl;
        emit serviceUrlChanged(serviceUrl);
    }
}

/*!
 * \brief Get the QNetworkAccessManager used by the Enginio library.
 *
 * Note that it will be deleted with the client object.
 */
QNetworkAccessManager * EnginioClient::networkManager() const
{
    Q_D(const EnginioClient);
    return d->networkManager();
}

/*!
 * \brief Create custom request to the enginio REST API
 *
 * \param url The url to be used for the request. Note that the provided url completely replaces the internal serviceUrl.
 * \param httpOperation Verb to the server that is valid according to the HTTP specification (eg. "GET", "POST", "PUT", etc.).
 * \param data optional JSON object possibly containing custom headers and the payload data for the request.
 *
 *   {
 *       "headers" : { "Accept" : "application/json" }
 *       "payload" : { "email": "me@engin.io", "password": "password" }
 *   }
 *
 * \return EnginioReply containing the status and the result once it is finished.
 * \sa EnginioReply, create(), query(), update(), remove()
 * \internal
 */
EnginioReply *EnginioClient::customRequest(const QUrl &url, const QByteArray &httpOperation, const QJsonObject &data)
{
    Q_D(EnginioClient);
    QNetworkReply *nreply = d->customRequest(url, httpOperation, data);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);
    return ereply;
}

/*!
  \brief Fulltext search on the Enginio backend

  The \a query is JSON sent to the backend to perform a fulltext search.
  Note that the search requires the searched properties to be indexed (on the server, configureable in the backend).

  \return EnginioReply containing the status and the result once it is finished.
  \sa EnginioReply, create(), query(), update(), remove()
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
  \brief Query the database.

  The \a query is a JSON object containing the actual query to the backend.
  The query will be run on the \a operation part of the backend.
  \return EnginioReply containing the status and the result once it is finished.
  \sa EnginioReply, create(), update(), remove(), Operation
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
  \snippet files/tst_files.cpp upload-create-object

  Then do the actual upload:
  \snippet files/tst_files.cpp upload

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

  \snippet files/tst_files.cpp download
  The propertyName can be anything, but it must be the same as the one used to upload the file with.
  This way one object can have several files attached to itself (one per propertyName).

  If a file provides several variants, it is possible to request a variant by including it in the
  \a object.
  \code
    {
        "id": "abc123",
        "variant": "thumbnail"
    }
  \endcode
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
    Q_ASSERT(!_networkManager);

    _networkManager = prepareNetworkManagerInThread();
    _networkManagerConnection = QObject::connect(_networkManager, &QNetworkAccessManager::finished, EnginioClientPrivate::ReplyFinishedFunctor(this));
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

EnginioClient::AuthenticationState EnginioClient::authenticationState() const
{
    Q_D(const EnginioClient);
    return d->authenticationState();
}
