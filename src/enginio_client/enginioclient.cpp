/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtEnginio module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
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
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <Enginio/private/enginioclient_p.h>
#include <Enginio/enginioreply.h>
#include <Enginio/private/enginioreply_p.h>
#include <Enginio/enginiomodel.h>
#include <Enginio/enginioidentity.h>
#include <Enginio/enginiooauth2authentication.h>

#include <QtCore/qthreadstorage.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>

QT_BEGIN_NAMESPACE

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
  By setting the \l EnginioClientConnection::backendId a backend is chosen.
  After setting the ID interaction with the server is possible.
  The information about the backend is available on the Enginio Dashboard
  after logging in to \l {http://engin.io}{Enginio}.
  \code
    EnginioClient *client = new EnginioClient(parent);
    client->setBackendId(QByteArrayLiteral("YOUR_BACKEND_ID"));
  \endcode

  The basic functions used to interact with the backend are
  \l create(), \l query(), \l remove() and \l update().
  It is possible to do a fulltext search on the server using \l fullTextSearch().
  For file handling \l downloadUrl() and \l uploadFile() are provided.
  The functions are asynchronous, which means that they are not blocking
  and the result of them will be delivered together with EnginioReply::finished()
  signal.

  \note After the request has finished, it is the responsibility of the
  user to delete the EnginioReply object at an appropriate time.
  Do not directly delete it inside the slot connected to finished().
  You can use the deleteLater() function.

  For a higher level overview, consult the \l {Enginio Overview} documentation.
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
  \property EnginioClientConnection::authenticationState
  \brief The state of the authentication.

  Enginio provides convenient user management.
  The authentication state reflects whether the current user is authenticated.
  \sa AuthenticationState EnginioClientConnection::identity() EnginioOAuth2Authentication
*/

/*!
  \fn EnginioClient::sessionAuthenticated(EnginioReply *reply) const
  \brief Emitted when a user logs in.

  The \a reply contains the details about the login.

  \sa sessionAuthenticationError(), EnginioReply, EnginioOAuth2Authentication
*/

/*!
  \fn EnginioClient::sessionAuthenticationError(EnginioReply *reply) const
  \brief Emitted when a user login fails.

  The \a reply contains the details about why the login failed.
  \sa sessionAuthenticated(), EnginioReply EnginioClientConnection::identity() EnginioOAuth2Authentication
*/

/*!
  \fn EnginioClient::sessionTerminated() const
  \brief Emitted when a user logs out.
  \sa EnginioClientConnection::identity() EnginioOAuth2Authentication
*/

/*!
    \enum EnginioClientConnection::Operation

    Enginio has a unified API for several \c operations.
    For example when using query(), the default is \c ObjectOperation,
    which means that the query will return objects from the database.
    When passing in UserOperation instead, the query will return
    users.

    \value ObjectOperation Operate on objects
    \value AccessControlOperation Operate on the ACL
    \value FileOperation Operate with files
    \value UserOperation Operate on users
    \value UsergroupOperation Operate on groups
    \value UsergroupMembersOperation Operate on group members
*/

/*!
    \enum EnginioClientConnection::AuthenticationState

    This enum describes the state of the user authentication.
    \value NotAuthenticated No attempt to authenticate was made
    \value Authenticating Authentication request has been sent to the server
    \value Authenticated Authentication was successful
    \value AuthenticationFailure Authentication failed

    \sa authenticationState
*/

ENGINIOCLIENT_EXPORT bool gEnableEnginioDebugInfo = !qEnvironmentVariableIsSet("ENGINIO_DEBUG_INFO");

QNetworkRequest EnginioClientConnectionPrivate::prepareRequest(const QUrl &url)
{
    QByteArray requestId = QUuid::createUuid().toByteArray();

    // Remove unneeded pretty-formatting.
    // before: "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"
    // after:  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
    requestId.chop(1);      // }
    requestId.remove(0, 1); // {
    requestId.remove(23, 1);
    requestId.remove(18, 1);
    requestId.remove(13, 1);
    requestId.remove(8, 1);

    QNetworkRequest req(_request);
    req.setUrl(url);
    req.setRawHeader(EnginioString::X_Request_Id, requestId);
    return req;
}

EnginioClientConnectionPrivate::EnginioClientConnectionPrivate() :
    _identity(),
    _serviceUrl(EnginioString::apiEnginIo),
    _networkManager(),
    _uploadChunkSize(512 * 1024),
    _authenticationState(EnginioClientConnection::NotAuthenticated)
{
    assignNetworkManager();

    _request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/json"));
}

void EnginioClientConnectionPrivate::init()
{
    QObject::connect(static_cast<EnginioClient*>(q_ptr), &EnginioClient::sessionTerminated, AuthenticationStateTrackerFunctor(this));
    QObject::connect(static_cast<EnginioClient*>(q_ptr), &EnginioClient::sessionAuthenticated, AuthenticationStateTrackerFunctor(this, EnginioClientConnection::Authenticated));
    QObject::connect(static_cast<EnginioClient*>(q_ptr), &EnginioClient::sessionAuthenticationError, AuthenticationStateTrackerFunctor(this, EnginioClientConnection::AuthenticationFailure));
}

void EnginioClientConnectionPrivate::replyFinished(QNetworkReply *nreply)
{
    EnginioReplyBase *ereply = _replyReplyMap.take(nreply);

    if (!ereply)
        return;

    if (nreply->error() != QNetworkReply::NoError) {
        QPair<QIODevice *, qint64> deviceState = _chunkedUploads.take(nreply);
        delete deviceState.first;
        emitError(ereply);
    }

    // continue chunked upload
    else if (_chunkedUploads.contains(nreply)) {
        QPair<QIODevice *, qint64> deviceState = _chunkedUploads.take(nreply);
        QString status = ereply->data().value(EnginioString::status).toString();
        if (status == EnginioString::empty || status == EnginioString::incomplete) {
            Q_ASSERT(ereply->data().value(EnginioString::objectType).toString() == EnginioString::files);
            uploadChunk(ereply, deviceState.first, deviceState.second);
            return;
        }
        // should never get here unless upload was successful
        Q_ASSERT(status == EnginioString::complete);
        delete deviceState.first;
        if (_connections.count() * 2 > _chunkedUploads.count()) {
            _connections.removeAll(QMetaObject::Connection());
        }
    }

    if (Q_UNLIKELY(ereply->delayFinishedSignal())) {
        // delay emittion of finished signal for autotests
        _delayedReplies.insert(ereply);
    } else {
        ereply->dataChanged();
        ereply->emitFinished();
        emitFinished(ereply);
        if (gEnableEnginioDebugInfo)
            _requestData.remove(nreply);
    }

    if (Q_UNLIKELY(_delayedReplies.count())) {
        finishDelayedReplies();
    }
}

bool EnginioClientConnectionPrivate::finishDelayedReplies()
{
    // search if we can trigger an old finished signal.
    bool needToReevaluate = false;
    do {
        needToReevaluate = false;
        foreach (EnginioReplyBase *reply, _delayedReplies) {
            if (!reply->delayFinishedSignal()) {
                reply->dataChanged();
                reply->emitFinished();
                emitFinished(reply);
                if (gEnableEnginioDebugInfo)
                    _requestData.remove(reply->d_func()->_nreply); // FIXME it is ugly, and breaks encapsulation
                _delayedReplies.remove(reply);
                needToReevaluate = true;
            }
        }
    } while (needToReevaluate);
    return !_delayedReplies.isEmpty();
}

EnginioClientConnectionPrivate::~EnginioClientConnectionPrivate()
{
    foreach (const QMetaObject::Connection &identityConnection, _identityConnections)
        QObject::disconnect(identityConnection);
    foreach (const QMetaObject::Connection &connection, _connections)
        QObject::disconnect(connection);
    QObject::disconnect(_networkManagerConnection);
}

class EnginioClientPrivate: public EnginioClientConnectionPrivate {
public:
    EnginioClientPrivate()
    {}
};

/*!
  \brief Creates a new EnginioClient with \a parent as QObject parent.
*/
EnginioClient::EnginioClient(QObject *parent)
    : EnginioClientConnection(*new EnginioClientPrivate, parent)
{
    Q_D(EnginioClient);
    d->init();
}

/*!
 * Destroys the EnginioClient.
 *
 * This ends the Enginio session.
 */
EnginioClient::~EnginioClient()
{}

/*!
 * \property EnginioClientConnection::backendId
 * \brief The unique ID for the used Enginio backend.
 *
 * The backend ID determines which Enginio backend is used
 * by this instance of EnginioClient. The backend ID is
 * required for Enginio to work.
 * It is possible to use several Enginio backends simultaneously
 * by having several instances of EnginioClient.
 */
QByteArray EnginioClientConnection::backendId() const
{
    Q_D(const EnginioClientConnection);
    return d->_backendId;
}

void EnginioClientConnection::setBackendId(const QByteArray &backendId)
{
    Q_D(EnginioClientConnection);
    if (d->_backendId != backendId) {
        d->_backendId = backendId;
        d->_request.setRawHeader("Enginio-Backend-Id", d->_backendId);
        emit backendIdChanged(backendId);
    }
}

/*!
  \property EnginioClientConnection::serviceUrl
  \brief Enginio backend URL.
  \internal

  The API URL determines the server used by Enginio.
  Usually it is not needed to change the default URL.
*/

/*!
  \fn EnginioClientConnection::serviceUrlChanged(const QUrl &url)
  \internal
*/

/*!
  \internal
*/
QUrl EnginioClientConnection::serviceUrl() const
{
    Q_D(const EnginioClientConnection);
    return d->_serviceUrl;
}

/*!
    \internal
*/
void EnginioClientConnection::setServiceUrl(const QUrl &serviceUrl)
{
    Q_D(EnginioClientConnection);
    if (d->_serviceUrl != serviceUrl) {
        d->_serviceUrl = serviceUrl;
        emit serviceUrlChanged(serviceUrl);
    }
}

/*!
  \brief Get the QNetworkAccessManager used by the Enginio library.

  \note that this QNetworkAccessManager may be shared with other EnginioClient instances
  and it is owned by them.
*/
QNetworkAccessManager *EnginioClientConnection::networkManager() const
{
    Q_D(const EnginioClientConnection);
    return d->networkManager();
}

/*!
  \brief Create custom request to the enginio REST API

  \a url The url to be used for the request. Note that the provided url completely replaces the internal serviceUrl.
  \a httpOperation Verb to the server that is valid according to the HTTP specification (eg. "GET", "POST", "PUT", etc.).
  \a data optional JSON object possibly containing custom headers and the payload data for the request.

    {
        "headers" : { "Accept" : "application/json" }
        "payload" : { "email": "me@engin.io", "password": "password" }
    }

  \return EnginioReply containing the status and the result once it is finished.
  \sa EnginioReply, create(), query(), update(), remove()
  \internal
 */
EnginioReply *EnginioClient::customRequest(const QUrl &url, const QByteArray &httpOperation, const QJsonObject &data)
{
    Q_D(EnginioClient);
    QNetworkReply *nreply = d->customRequest(url, httpOperation, data);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    return ereply;
}

/*!
  \brief Fulltext search on the Enginio backend

  The \a query is JSON sent to the backend to perform a fulltext search.
  Note that the search requires the searched properties to be indexed (on the server, configureable in the backend).

  \return EnginioReply containing the status and the result once it is finished.
  \sa EnginioReply, create(), query(), update(), remove()
*/
EnginioReply *EnginioClient::fullTextSearch(const QJsonObject &query)
{
    Q_D(EnginioClient);

    QNetworkReply *nreply = d->query<QJsonObject>(query, EnginioClientConnectionPrivate::SearchOperation);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    return ereply;
}

/*!
  \brief Query the database.

  The \a query is a JSON object containing the actual query to the backend.
  The query will be run on the \a operation part of the backend.

  To query the database of all objects of type "objects.todo":
  \snippet enginioclient/tst_enginioclient.cpp query-todo

  \return EnginioReply containing the status and the result once it is finished.
  \sa EnginioReply, create(), update(), remove(), EnginioClientConnection::Operation
 */
EnginioReply* EnginioClient::query(const QJsonObject &query, const Operation operation)
{
    Q_D(EnginioClient);

    QNetworkReply *nreply = d->query<QJsonObject>(query, static_cast<EnginioClientConnectionPrivate::Operation>(operation));
    EnginioReply *ereply = new EnginioReply(d, nreply);

    return ereply;
}

/*!
  \brief Insert a new \a object into the database.

  The \a operation is the area in which the object gets created. It defaults to \l EnginioClientConnection::ObjectOperation
  to create new objects by default.

  \snippet enginioclient/tst_enginioclient.cpp create-todo

  To add a new member to a usergroup, the JSON needs to look like the example below.
  \code
  {
      "id": "groupId",
      "member": { "id": "abcd", "objectType": "users" }
  }
  \endcode
  It can be constructed like this:
  \snippet enginioclient/tst_enginioclient.cpp create-newmember

  \return EnginioReply containing the status of the query and the data once it is finished.
  \sa EnginioReply, query(), update(), remove()
*/
EnginioReply* EnginioClient::create(const QJsonObject &object, const Operation operation)
{
    Q_D(EnginioClient);

    QNetworkReply *nreply = d->create<QJsonObject>(object, operation);
    EnginioReply *ereply = new EnginioReply(d, nreply);

    return ereply;
}

/*!
  \brief Update an existing \a object in the database.

  The \a operation is the area in which the object gets created. It defaults to \l EnginioClientConnection::ObjectOperation
  to create new objects by default.

  To update access control list of an object the JSON loook like this:
  \code
  {
        "id": "objectId",
        "objectType": "objects.objectType",
        "access": { "read": ["id": "userId", "objectTypes": "users"],
                     "update": ["id": "userId", "objectTypes": "users"],
                     "admin": ["id": "userId", "objectTypes": "users"] }
  }
  \endcode
  which could be implemented for example this way:
  \snippet enginioclient/tst_enginioclient.cpp update-access

  \return EnginioReply containing the status of the query and the data once it is finished.
  \sa EnginioReply, create(), query(), remove()
*/
EnginioReply* EnginioClient::update(const QJsonObject &object, const Operation operation)
{
    Q_D(EnginioClient);

    QNetworkReply *nreply = d->update<QJsonObject>(object, operation);
    EnginioReply *ereply = new EnginioReply(d, nreply);

    return ereply;
}

/*!
  \brief Remove an existing \a object from the database.

  The \a operation is the area in which the object gets created. It defaults to \l EnginioClientConnection::ObjectOperation
  to create new objects by default.

  \snippet enginioclient/tst_enginioclient.cpp remove-todo

  \return EnginioReply containing the status of the query and the data once it is finished.
  \sa EnginioReply, create(), query(), update()
*/
EnginioReply* EnginioClient::remove(const QJsonObject &object, const Operation operation)
{
    Q_D(EnginioClient);

    QNetworkReply *nreply = d->remove<QJsonObject>(object, operation);
    EnginioReply *ereply = new EnginioReply(d, nreply);

    return ereply;
}

/*!
  \property EnginioClientConnection::identity
  Property that represents a user. Setting the property will create an asynchronous authentication request,
  the result of it updates \l{EnginioClientConnection::authenticationState()}{authenticationState}
  EnginioClient does not take ownership of the \a identity object. The object
  has to be valid to keep the authenticated session alive. When the identity object is deleted the session
  is terminated. It is allowed to assign a null pointer to the property to terminate the session.
  \sa EnginioIdentity EnginioOAuth2Authentication
*/
EnginioIdentity *EnginioClientConnection::identity() const
{
    Q_D(const EnginioClientConnection);
    return d->identity();
}

void EnginioClientConnection::setIdentity(EnginioIdentity *identity)
{
    Q_D(EnginioClientConnection);
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

  \sa downloadUrl()
*/
EnginioReply* EnginioClient::uploadFile(const QJsonObject &object, const QUrl &file)
{
    Q_D(EnginioClient);

    QNetworkReply *nreply = d->uploadFile<QJsonObject>(object, file);
    EnginioReply *ereply = new EnginioReply(d, nreply);

    return ereply;
}

/*!
  \brief Get a temporary URL for a file stored in Enginio

  From this URL a file can be downloaded. The URL is valid for a certain amount of time as indicated
  in the reply.

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
EnginioReply* EnginioClient::downloadUrl(const QJsonObject &object)
{
    Q_D(EnginioClient);

    QNetworkReply *nreply = d->downloadUrl<QJsonObject>(object);
    EnginioReply *ereply = new EnginioReply(d, nreply);

    return ereply;
}

Q_GLOBAL_STATIC(QThreadStorage<QWeakPointer<QNetworkAccessManager> >, NetworkManager)

void EnginioClientConnectionPrivate::assignNetworkManager()
{
    Q_ASSERT(!_networkManager);

    _networkManager = prepareNetworkManagerInThread();
    _networkManagerConnection = QObject::connect(_networkManager.data(), &QNetworkAccessManager::finished, EnginioClientConnectionPrivate::ReplyFinishedFunctor(this));
}

QSharedPointer<QNetworkAccessManager> EnginioClientConnectionPrivate::prepareNetworkManagerInThread()
{
    QSharedPointer<QNetworkAccessManager> qnam;
    qnam = NetworkManager->localData().toStrongRef();
    if (!qnam) {
        qnam = QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager());
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0) && !defined(QT_NO_SSL)
        qnam->connectToHostEncrypted(EnginioString::apiEnginIo);
#endif
        NetworkManager->setLocalData(qnam);
    }
    return qnam;
}

EnginioClientConnection::AuthenticationState EnginioClientConnection::authenticationState() const
{
    Q_D(const EnginioClientConnection);
    return d->authenticationState();
}

/*!
  \internal
  Tries to emit finished signal from all replies that used to be delayed.
  \return false if all replies were finished, true otherwise.
*/
bool EnginioClientConnection::finishDelayedReplies()
{
    Q_D(EnginioClientConnection);
    return d->finishDelayedReplies();
}

/*!
  \internal
*/
EnginioClientConnection::EnginioClientConnection(EnginioClientConnectionPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
    qRegisterMetaType<EnginioClient*>();
    qRegisterMetaType<EnginioModel*>();
    qRegisterMetaType<EnginioReply*>();
    qRegisterMetaType<EnginioIdentity*>();
    qRegisterMetaType<EnginioOAuth2Authentication*>();
    qRegisterMetaType<EnginioClientConnection::Operation>();
    qRegisterMetaType<EnginioClientConnection::AuthenticationState>();
}

EnginioClientConnection::~EnginioClientConnection()
{
    qDeleteAll(findChildren<EnginioReplyBase *>());
}

void EnginioClientConnectionPrivate::emitSessionTerminated() const
{
    emit static_cast<EnginioClient*>(q_ptr)->sessionTerminated();
}

void EnginioClientConnectionPrivate::emitSessionAuthenticated(EnginioReplyBase *reply)
{
    emit static_cast<EnginioClient*>(q_ptr)->sessionAuthenticated(static_cast<EnginioReply*>(reply));
}

void EnginioClientConnectionPrivate::emitSessionAuthenticationError(EnginioReplyBase *reply)
{
    emit static_cast<EnginioClient*>(q_ptr)->sessionAuthenticationError(static_cast<EnginioReply*>(reply));
}

void EnginioClientConnectionPrivate::emitFinished(EnginioReplyBase *reply)
{
    emit static_cast<EnginioClient*>(q_ptr)->finished(static_cast<EnginioReply*>(reply));
}

void EnginioClientConnectionPrivate::emitError(EnginioReplyBase *reply)
{
    emit static_cast<EnginioClient*>(q_ptr)->error(static_cast<EnginioReply*>(reply));
}

EnginioReplyBase *EnginioClientConnectionPrivate::createReply(QNetworkReply *nreply)
{
    return new EnginioReply(this, nreply);
}

QByteArray EnginioClientConnectionPrivate::constructErrorMessage(const QByteArray &msg)
{
    static QByteArray msgBegin = QByteArrayLiteral("{\"errors\": [{\"message\": \"");
    static QByteArray msgEnd = QByteArrayLiteral("\",\"reason\": \"BadRequest\"}]}");
    return msgBegin + msg + msgEnd;
}

QT_END_NAMESPACE
