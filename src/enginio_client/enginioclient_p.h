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

#ifndef ENGINIOCLIENT_P_H
#define ENGINIOCLIENT_P_H

#include "chunkdevice_p.h"
#include "enginioclient.h"
#include "enginioreply.h"
#include "enginiofakereply_p.h"
#include "enginioidentity.h"
#include "enginioobjectadaptor_p.h"

#include <QNetworkAccessManager>
#include <QPointer>
#include <QUrl>
#include <QtCore/qjsondocument.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qhttpmultipart.h>
#include <QtCore/qurlquery.h>
#include <QtCore/qfile.h>
#include <QtCore/qmimedatabase.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qbuffer.h>
#include <QtCore/qlinkedlist.h>

#define FOR_EACH_ENGINIO_STRING(F)\
    F(_synced, "_synced")\
    F(access, "access")\
    F(apiEnginIo, "https://api.engin.io")\
    F(authIdentity, "auth/identity")\
    F(complete, "complete")\
    F(count, "count")\
    F(createdAt, "createdAt")\
    F(empty, "empty")\
    F(file, "file")\
    F(fileName, "fileName")\
    F(files, "files")\
    F(headers, "headers")\
    F(id, "id")\
    F(include, "include")\
    F(incomplete, "incomplete")\
    F(limit, "limit")\
    F(members, "members")\
    F(message, "message")\
    F(object, "object")\
    F(objectType, "objectType")\
    F(objectTypes, "objectTypes")\
    F(offset, "offset")\
    F(pageSize, "pageSize")\
    F(password, "password")\
    F(payload, "payload")\
    F(propertyName, "propertyName")\
    F(query, "query")\
    F(results, "results")\
    F(search, "search")\
    F(session, "session")\
    F(sessionToken, "sessionToken")\
    F(sort, "sort")\
    F(status, "status")\
    F(targetFileProperty, "targetFileProperty")\
    F(updatedAt, "updatedAt")\
    F(url, "url")\
    F(usergroups, "usergroups")\
    F(username, "username")\
    F(users, "users")\
    F(variant, "variant")\


struct ENGINIOCLIENT_EXPORT EnginioString
{
#define DECLARE_ENGINIO_STRING(Name, String)\
    static const QString Name;

    FOR_EACH_ENGINIO_STRING(DECLARE_ENGINIO_STRING)
#undef DECLARE_ENGINIO_STRING
};


#define CHECK_AND_SET_URL_PATH_IMPL(Url, Object, Operation, Flags) \
    {\
        QString _path; \
        QByteArray _errorMsg; \
        if (!getPath(Object, Operation, &_path, &_errorMsg, Flags)) \
            return new EnginioFakeReply(this, _errorMsg); \
        Url.setPath(_path); \
    }

#define CHECK_AND_SET_PATH(Url, Object, Operation) \
    CHECK_AND_SET_URL_PATH_IMPL(Url, Object, Operation, EnginioClientPrivate::Default)

#define CHECK_AND_SET_PATH_WITH_ID(Url, Object, Operation) \
    CHECK_AND_SET_URL_PATH_IMPL(Url, Object, Operation, EnginioClientPrivate::IncludeIdInPath)

static QByteArray constructErrorMessage(QByteArray msg)
{
    static QByteArray msgBegin = QByteArrayLiteral("{\"errors\": [{\"message\": \"");
    static QByteArray msgEnd = QByteArrayLiteral("\",\"reason\": \"BadRequest\"}]}");
    return msgBegin + msg + msgEnd;
}

class ENGINIOCLIENT_EXPORT EnginioClientPrivate
{
    enum PathOptions { Default, IncludeIdInPath = 1};


    template<class T>
    static bool getPath(const T &object, int operation, QString *path, QByteArray *errorMsg, PathOptions flags = Default)
    {
        QByteArray &msg = *errorMsg;

        QString &result = *path;
        result.reserve(96);
        result.append(QStringLiteral("/v1/"));

        switch (operation) {
        case ObjectOperation: {
            QString objectType = object[EnginioString::objectType].toString();
            if (objectType.isEmpty()) {
                msg = constructErrorMessage(QByteArrayLiteral("Requested object operation requires non empty \'objectType\' value"));
                return false;
            }

            result.append(objectType.replace('.', '/'));
            break;
        }
        case ObjectAclOperation:
        {
            QString objectType = object[EnginioString::objectType].toString();
            if (objectType.isEmpty()) {
                msg = constructErrorMessage(QByteArrayLiteral("Requested object acl operation requires non empty \'objectType\' value"));
                return false;
            }

            result.append(objectType.replace('.', '/'));
            QString id = object[EnginioString::id].toString();
            if (id.isEmpty()) {
                msg = constructErrorMessage(QByteArrayLiteral("Requested object acl operation requires non empty \'id\' value"));
                return false;
            }
            result.append('/');
            result.append(id);
            result.append('/');
            result.append(EnginioString::access);
            return true;
        }
        case AuthenticationOperation:
            result.append(EnginioString::authIdentity);
            break;
        case FileOperation: {
            result.append(EnginioString::files);
            // if we have a fileID, it becomes "view", otherwise it is up/download
            QString fileId = object[EnginioString::id].toString();
            if (!fileId.isEmpty()) {
                result.append('/');
                result.append(fileId);
            }
            break;
        }
        case FileGetDownloadUrlOperation: {
            result.append(EnginioString::files);
            QString fileId = object[EnginioString::id].toString();
            if (fileId.isEmpty()) {
                msg = constructErrorMessage(QByteArrayLiteral("Download operation requires non empty \'fileId\' value"));
                return false;
            }
            result.append(QLatin1Char('/') + fileId + QStringLiteral("/download_url"));
            break;
        }
        case FileChunkUploadOperation: {
            const QString fileId = object[EnginioString::id].toString();
            Q_ASSERT(!fileId.isEmpty());
            result.append(EnginioString::files + QLatin1Char('/') + fileId + QStringLiteral("/chunk"));
            break;
        }
        case SearchOperation:
            result.append(EnginioString::search);
            break;
        case SessionOperation:
            result.append(EnginioString::session);
            break;
        case UserOperation:
            result.append(EnginioString::users);
            break;
        case UsergroupOperation:
            result.append(EnginioString::usergroups);
            break;
        case UsergroupMemberOperation:
        {
            QString id = object[EnginioString::id].toString();
            if (id.isEmpty()) {
                msg = constructErrorMessage(QByteArrayLiteral("Requested usergroup member operation requires non empty \'id\' value"));
                return false;
            }
            result.append(EnginioString::usergroups);
            result.append('/');
            result.append(id);
            result.append('/');
            result.append(EnginioString::members);
            return true;
        }
        }

        if (flags & IncludeIdInPath) {
            QString id = object[EnginioString::id].toString();
            if (id.isEmpty()) {
                msg = constructErrorMessage(QByteArrayLiteral("Requested operation requires non empty \'id\' value"));
                return false;
            }
            result.append('/');
            result.append(id);
        }

        return true;
    }

    class ReplyFinishedFunctor
    {
        EnginioClientPrivate *d;

    public:
        ReplyFinishedFunctor(EnginioClientPrivate *enginio)
            : d(enginio)
        {
            Q_ASSERT(d);
        }

        void operator ()(QNetworkReply *nreply)
        {
            d->replyFinished(nreply);
        }
    };

    class CallPrepareSessionToken
    {
        EnginioClientPrivate *_enginio;
        EnginioIdentity *_identity;

    public:
        CallPrepareSessionToken(EnginioClientPrivate *enginio, EnginioIdentity *identity)
            : _enginio(enginio)
            , _identity(identity)
        {}
        void operator ()()
        {
            if (!_enginio->_backendId.isEmpty() && !_enginio->_backendSecret.isEmpty()) {
                // TODO should we disconnect backendId and backendSecret change singals?
                _identity->prepareSessionToken(_enginio);
            }
        }
    };

    class IdentityInstanceDestroyed
    {
        EnginioClientPrivate *_enginio;

    public:
        IdentityInstanceDestroyed(EnginioClientPrivate *enginio)
            : _enginio(enginio)
        {}
        void operator ()()
        {
            _enginio->setIdentity(0);
        }
    };

    class AuthenticationStateTrackerFunctor
    {
        EnginioClientPrivate *_enginio;
        EnginioClient::AuthenticationState _state;
    public:
        AuthenticationStateTrackerFunctor(EnginioClientPrivate *enginio, EnginioClient::AuthenticationState state = EnginioClient::NotAuthenticated)
            : _enginio(enginio)
            , _state(state)
        {}

        void operator()() const
        {
            _enginio->setAuthenticationState(_state);
        }
    };

    class AuthenticationStateTrackerIdentFunctor
    {
        EnginioClientPrivate *_enginio;
    public:
        AuthenticationStateTrackerIdentFunctor(EnginioClientPrivate *enginio)
            : _enginio(enginio)
        {}

        void operator()(const EnginioIdentity *identity) const
        {
            if (identity)
                _enginio->setAuthenticationState(EnginioClient::Authenticating);
            // else we could call _enginio->setAuthenticationState(EnginioClient::NotAuthenticated);
            // but sessionTerminated signal should do the trick anyway
        }
    };

public:
    enum Operation {
        // Do not forget to keep in sync with EnginioClient::Operation!
        ObjectOperation = EnginioClient::ObjectOperation,
        ObjectAclOperation = EnginioClient::ObjectAclOperation,
        UserOperation = EnginioClient::UserOperation,
        UsergroupOperation = EnginioClient::UsergroupOperation,
        UsergroupMemberOperation = EnginioClient::UsergroupMembersOperation,
        FileOperation = EnginioClient::FileOperation,

        // private
        AuthenticationOperation,
        SessionOperation,
        SearchOperation,
        FileChunkUploadOperation,
        FileGetDownloadUrlOperation
    };

    Q_ENUMS(Operation)

    EnginioClientPrivate(EnginioClient *client = 0);
    virtual ~EnginioClientPrivate();
    static EnginioClientPrivate* get(EnginioClient *client) { return client->d_func(); }
    static const EnginioClientPrivate* get(const EnginioClient *client) { return client->d_func(); }


    EnginioClient *q_ptr;
    QByteArray _backendId;
    QByteArray _backendSecret;
    EnginioIdentity *_identity;

    QLinkedList<QMetaObject::Connection> _connections;
    QVarLengthArray<QMetaObject::Connection, 4> _identityConnections;
    QUrl _serviceUrl;
    QNetworkAccessManager *_networkManager;
    QMetaObject::Connection _networkManagerConnection;
    QNetworkRequest _request;
    QMap<QNetworkReply*, EnginioReply*> _replyReplyMap;
    QMap<QNetworkReply*, QByteArray> _requestData;

    // device and last position
    QMap<QNetworkReply*, QPair<QIODevice*, qint64> > _chunkedUploads;
    qint64 _uploadChunkSize;
    QJsonObject _identityToken;
    EnginioClient::AuthenticationState _authenticationState;

    QSet<EnginioReply*> _delayedReplies; // Used only for testing

    void init();

    void replyFinished(QNetworkReply *nreply);

    void setAuthenticationState(const EnginioClient::AuthenticationState state)
    {
        if (_authenticationState == state)
            return;
        _authenticationState = state;
        emit q_ptr->authenticationStateChanged(state);
    }

    EnginioClient::AuthenticationState authenticationState() const
    {
        return _authenticationState;
    }

    QJsonObject identityToken() const
    {
        return _identityToken;
    }

    void setIdentityToken(EnginioReply *reply)
    {
        QByteArray sessionToken;
        if (reply) {
            _identityToken = reply->data();
            sessionToken = _identityToken[EnginioString::sessionToken].toString().toLatin1();
        }

        _request.setRawHeader(QByteArrayLiteral("Enginio-Backend-Session"), sessionToken);
        if (sessionToken.isEmpty())
            emit q_ptr->sessionTerminated();
        else
            emit q_ptr->sessionAuthenticated(reply);
    }

    void registerReply(QNetworkReply *nreply, EnginioReply *ereply)
    {
        _replyReplyMap[nreply] = ereply;
    }

    EnginioIdentity *identity() const
    {
        return _identity;
    }

    void setIdentity(EnginioIdentity *identity)
    {
        foreach (const QMetaObject::Connection &identityConnection, _identityConnections)
            QObject::disconnect(identityConnection);
        _identityConnections.clear();

        if (!(_identity = identity)) {
            // invalidate old identity token
            setIdentityToken(0);
            return;
        }
        CallPrepareSessionToken callPrepareSessionToken(this, identity);
        if (_backendId.isEmpty() || _backendSecret.isEmpty()) {
            if (_backendId.isEmpty())
                _identityConnections.append(QObject::connect(q_ptr, &EnginioClient::backendIdChanged, callPrepareSessionToken));
            if (_backendSecret.isEmpty())
                _identityConnections.append(QObject::connect(q_ptr, &EnginioClient::backendSecretChanged, callPrepareSessionToken));
        } else
            identity->prepareSessionToken(this);
        _identityConnections.append(QObject::connect(identity, &EnginioIdentity::dataChanged, callPrepareSessionToken));
        _identityConnections.append(QObject::connect(identity, &EnginioIdentity::destroyed, IdentityInstanceDestroyed(this)));
        emit q_ptr->identityChanged(identity);
    }

    QNetworkReply *identify(const QJsonObject &object)
    {
        QUrl url(_serviceUrl);
        CHECK_AND_SET_PATH(url, object, AuthenticationOperation);

        QNetworkRequest req(_request);
        req.setUrl(url);
        QByteArray data(QJsonDocument(object).toJson(QJsonDocument::Compact));
        QNetworkReply *reply = networkManager()->post(req, data);

        if (gEnableEnginioDebugInfo)
            _requestData.insert(reply, data);

        return reply;
    }

    QNetworkReply *customRequest(const QUrl &url, const QByteArray &httpOperation, const QJsonObject &data)
    {
        Q_ASSERT(!url.isEmpty());
        Q_ASSERT(!httpOperation.isEmpty());

        QNetworkRequest req(_request);
        req.setUrl(url);

        if (data[EnginioString::headers].isObject()) {
            QJsonObject headers = data[EnginioString::headers].toObject();

            QJsonObject::const_iterator end = headers.constEnd();
            for (QJsonObject::const_iterator i = headers.constBegin(); i != end; i++) {
                QByteArray headerName = i.key().toUtf8();
                QByteArray headerValue = i.value().toString().toUtf8();
                req.setRawHeader(headerName, headerValue);
            }
        }

        QBuffer *buffer = 0;
        QByteArray payload;

        if (data[EnginioString::payload].isObject()) {
            ObjectAdaptor<QJsonObject> o(data[EnginioString::payload].toObject());
            payload = o.toJson();
            buffer = new QBuffer();
            buffer->setData(payload);
            buffer->open(QIODevice::ReadOnly);
        }

        QNetworkReply *reply = networkManager()->sendCustomRequest(req, httpOperation, buffer);

        if (gEnableEnginioDebugInfo && !payload.isEmpty())
            _requestData.insert(reply, payload);

        if (buffer)
            buffer->setParent(reply);

        return reply;
    }

    template<class T>
    QNetworkReply *update(const ObjectAdaptor<T> &object, const EnginioClient::Operation operation)
    {
        QUrl url(_serviceUrl);
        CHECK_AND_SET_PATH_WITH_ID(url, object, operation);

        QNetworkRequest req(_request);
        req.setUrl(url);

        // TODO FIXME we need to remove "id" and "objectType" because of an internal server error.
        // It failes at least for ACL but maybe for others too.
        // Sadly we need to detach here, so it causes at least one allocation. Of course sending
        // garbage is also wrong so maybe we should filter out data before sending, the question
        // is how, and it seems that only enginio server knows...
        // TODO It would work only for QJSON classes, QJSValue is a reference to real object therefore
        // copy constructor will not detach, we are altering original object!
        ObjectAdaptor<T> o(object);
        o.remove(EnginioString::objectType);
        o.remove(EnginioString::id);
        QByteArray data = o.toJson();

        QNetworkReply *reply = networkManager()->put(req, data);

        if (gEnableEnginioDebugInfo)
            _requestData.insert(reply, data);

        return reply;
    }

    template<class T>
    QNetworkReply *remove(const ObjectAdaptor<T> &object, const EnginioClient::Operation operation)
    {
        QUrl url(_serviceUrl);
        CHECK_AND_SET_PATH_WITH_ID(url, object, operation);

        QNetworkRequest req(_request);
        req.setUrl(url);

        // TODO FIXME we need to remove "id" and "objectType" because of an internal server error.
        // It failes at least for ACL but maybe for others too.
        // Sadly we need to detach here, so it causes at least one allocation. Of course sending
        // garbage is also wrong so maybe we should filter out data before sending, the question
        // is how, and it seems that only enginio server knows...
        // TODO It would work only for QJSON classes, QJSValue is a reference to real object therefore
        // copy constructor will not detach, we are altering original object!
        ObjectAdaptor<T> o(object);
        o.remove(EnginioString::objectType);
        o.remove(EnginioString::id);
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
        if (operation == EnginioClient::ObjectAclOperation) {
            QByteArray data = o.toJson();
            QBuffer *buffer = new QBuffer();
            buffer->setData(data);
            buffer->open(QIODevice::ReadOnly);
            QNetworkReply *reply = networkManager()->sendCustomRequest(req, QByteArrayLiteral("DELETE"), buffer);
            buffer->setParent(reply);

            if (gEnableEnginioDebugInfo)
                _requestData.insert(reply, data);

            return reply;
        }
        return networkManager()->deleteResource(req);
#else
        QByteArray data = o.toJson();
        QNetworkReply *reply = networkManager()->deleteResource(req, data);

        if (gEnableEnginioDebugInfo)
            _requestData.insert(reply, data);

        return reply;
#endif
    }

    template<class T>
    QNetworkReply *create(const ObjectAdaptor<T> &object, const EnginioClient::Operation operation)
    {
        QUrl url(_serviceUrl);
        CHECK_AND_SET_PATH(url, object, operation);

        QNetworkRequest req(_request);
        req.setUrl(url);

        QByteArray data = object.toJson();

        QNetworkReply *reply = networkManager()->post(req, data);

        if (gEnableEnginioDebugInfo)
            _requestData.insert(reply, data);

        return reply;
    }

    template<class T>
    QNetworkReply *query(const ObjectAdaptor<T> &object, const Operation operation)
    {
        QUrl url(_serviceUrl);
        CHECK_AND_SET_PATH(url, object, operation);

        // TODO add all params here
        QUrlQuery urlQuery;
        if (int limit = object[EnginioString::limit].toInt()) {
            urlQuery.addQueryItem(EnginioString::limit, QString::number(limit));
        }
        if (int offset = object[EnginioString::offset].toInt()) {
            urlQuery.addQueryItem(EnginioString::offset, QString::number(offset));
        }
        if (object.contains(EnginioString::count)) { // TODO docs are saying about integer but it is not interpreted.
            urlQuery.addQueryItem(EnginioString::count, QString(0, Qt::Uninitialized));
        }
        ValueAdaptor<T> include = object[EnginioString::include];
        if (include.isComposedType()) {
            urlQuery.addQueryItem(EnginioString::include,
                QString::fromUtf8(include.toJson()));
        }
        ValueAdaptor<T> sort = object[EnginioString::sort];
        if (sort.isComposedType()) {
            urlQuery.addQueryItem(EnginioString::sort,
                QString::fromUtf8(sort.toJson()));
        }
        if (operation == SearchOperation) {
            ValueAdaptor<T> search = object[EnginioString::search];
            ArrayAdaptor<T> objectTypes = object[EnginioString::objectTypes].toArray();
            if (search.isComposedType()) {
                for (typename ArrayAdaptor<T>::const_iterator i = objectTypes.constBegin(); i != objectTypes.constEnd(); ++i) {
                    urlQuery.addQueryItem(QStringLiteral("objectTypes[]"), (*i).toString());
                }
                urlQuery.addQueryItem(EnginioString::search,
                    QString::fromUtf8(search.toJson()));
            } else {
                return new EnginioFakeReply(this, constructErrorMessage(QByteArrayLiteral("Fulltext Search: 'search' parameter(s) missing")));
            }
        } else
        if (object[EnginioString::query].isComposedType()) { // TODO docs are inconsistent on that
            urlQuery.addQueryItem(QStringLiteral("q"),
                QString::fromUtf8(object[EnginioString::query].toJson()));
        }
        url.setQuery(urlQuery);

        QNetworkRequest req(_request);
        req.setUrl(url);

        return networkManager()->get(req);
    }

    template<class T>
    QNetworkReply *downloadFile(const ObjectAdaptor<T> &object)
    {
        QUrl url(_serviceUrl);
        CHECK_AND_SET_PATH(url, object, FileGetDownloadUrlOperation);
        if (object.contains(EnginioString::variant)) {
            QString variant = object[EnginioString::variant].toString();
            QUrlQuery query;
            query.addQueryItem(EnginioString::variant, variant);
            url.setQuery(query);
        }

        QNetworkRequest req(_request);
        req.setUrl(url);

        QNetworkReply *reply = networkManager()->get(req);
        return reply;
    }

    template<class T>
    QNetworkReply *uploadFile(const ObjectAdaptor<T> &object, const QUrl &fileUrl)
    {
        if (!fileUrl.scheme().isEmpty() && !fileUrl.isLocalFile())
            qWarning() << "Enginio: Upload must be local file.";
        QString path = fileUrl.isLocalFile() ? fileUrl.toLocalFile() : fileUrl.path();

        QFile *file = new QFile(path);
        if (!file->exists()) {
            QByteArray msg = QByteArray("Cannot upload a not existing file ('") + path.toUtf8() + QByteArray("')");
            msg = constructErrorMessage(msg);
            delete file;
            return new EnginioFakeReply(this, msg);
        }

        if (!file->open(QFile::ReadOnly)) {
            QByteArray msg = QByteArray("File ('") + path.toUtf8() + QByteArray("') could not be opened for reading");
            msg = constructErrorMessage(msg);
            delete file;
            return new EnginioFakeReply(this, msg);
        }
        QMimeDatabase mimeDb;
        QString mimeType = mimeDb.mimeTypeForFile(path).name();
        return upload(object, file, mimeType);
    }

    template<class T>
    QNetworkReply *upload(const ObjectAdaptor<T> &object, QIODevice *device, const QString &mimeType)
    {
        QNetworkReply *reply = 0;
        if (!device->isSequential() && device->size() < _uploadChunkSize)
            reply = uploadAsHttpMultiPart(object, device, mimeType);
        else
            reply = uploadChunked(object, device);

        if (gEnableEnginioDebugInfo) {
            QByteArray data = object.toJson();
            _requestData.insert(reply, data);
        }

        return reply;
    }

    QNetworkAccessManager *networkManager() const
    {
        return _networkManager;
    }

    void assignNetworkManager();
    static QNetworkAccessManager *prepareNetworkManagerInThread();

    bool isSignalConnected(const QMetaMethod &signal) const
    {
        return q_ptr->isSignalConnected(signal);
    }

    class UploadProgressFunctor
    {
    public:
        UploadProgressFunctor(EnginioClientPrivate *client, QNetworkReply *reply)
            : _client(client), _reply(reply)
        {
            Q_ASSERT(_client);
            Q_ASSERT(_reply);
        }

        void operator ()(qint64 progress, qint64 total)
        {
            EnginioReply *ereply = _client->_replyReplyMap.value(_reply);
            qint64 p = progress;
            qint64 t = total;
            if (_client->_chunkedUploads.contains(_reply)) {
                QPair<QIODevice*, qint64> chunkData = _client->_chunkedUploads.value(_reply);
                t = chunkData.first->size();
                p += chunkData.second;
            }
            emit ereply->progress(p, t);
        }
    private:
        EnginioClientPrivate *_client;
        QNetworkReply *_reply;
    };

private:

    template<class T>
    QNetworkReply *uploadAsHttpMultiPart(const ObjectAdaptor<T> &object, QIODevice *device, const QString &mimeType)
    {
        QUrl serviceUrl = _serviceUrl;
        CHECK_AND_SET_PATH(serviceUrl, QJsonObject(), FileOperation);

        QNetworkRequest req(_request);
        req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray());
        req.setUrl(serviceUrl);

        QHttpMultiPart *multiPart = createHttpMultiPart(object, device, mimeType);
        QNetworkReply *reply = networkManager()->post(req, multiPart);
        multiPart->setParent(reply);
        device->setParent(multiPart);
        _connections.append(QObject::connect(reply, &QNetworkReply::uploadProgress, UploadProgressFunctor(this, reply)));
        return reply;
    }


    /* Create a multi part upload:
     * That means the JSON metadata and the actual file get sent in one http-post.
     * The associatedObject has to be a valid object type on the server.
     * If it does not contain an id, it needs to be manually associated later or will get garbage collected eventually.
     */
    template<class T>
    QHttpMultiPart *createHttpMultiPart(const ObjectAdaptor<T> &object, QIODevice *data, const QString &mimeType)
    {
        // check file/chunk size
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        data->setParent(multiPart);

        QHttpPart objectPart;
        objectPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                             QStringLiteral("form-data; name=\"object\""));

        objectPart.setBody(object.toJson());
        multiPart->append(objectPart);

        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QStringLiteral("form-data; name=\"file\"; filename=\"%1\"").arg(object[EnginioString::file].toObject()[EnginioString::fileName].toString()));
        filePart.setBodyDevice(data);
        multiPart->append(filePart);
        return multiPart;
    }

    template<class T>
    QNetworkReply *uploadChunked(const ObjectAdaptor<T> &object, QIODevice *device)
    {
        QUrl serviceUrl = _serviceUrl;
        CHECK_AND_SET_PATH(serviceUrl, QJsonObject(), FileOperation);

        QNetworkRequest req(_request);
        req.setUrl(serviceUrl);

        QNetworkReply *reply = networkManager()->post(req, object.toJson());
        _chunkedUploads.insert(reply, qMakePair(device, static_cast<qint64>(0)));
        _connections.append(QObject::connect(reply, &QNetworkReply::uploadProgress, UploadProgressFunctor(this, reply)));
        return reply;
    }

    void uploadChunk(EnginioReply *ereply, QIODevice *device, qint64 startPos)
    {
        QUrl serviceUrl = _serviceUrl;
        {
            QString path;
            QByteArray errorMsg;
            if (!getPath(ereply->data(), FileChunkUploadOperation, &path, &errorMsg))
                Q_UNREACHABLE(); // sequential upload can not have an invalid path!
            serviceUrl.setPath(path);
        }

        QNetworkRequest req(_request);
        req.setUrl(serviceUrl);
        req.setHeader(QNetworkRequest::ContentTypeHeader,
                      QByteArrayLiteral("application/octet-stream"));

        // Content-Range: bytes {chunkStart}-{chunkEnd}/{totalFileSize}
        qint64 size = device->size();
        qint64 endPos = qMin(startPos + _uploadChunkSize, size);
        req.setRawHeader(QByteArrayLiteral("Content-Range"),
                         QByteArray::number(startPos) + QByteArrayLiteral("-")
                         + QByteArray::number(endPos) + QByteArrayLiteral("/")
                         + QByteArray::number(size));

        // qDebug() << "Uploading chunk from " << startPos << " to " << endPos << " of " << size;

        Q_ASSERT(device->isOpen());

        ChunkDevice *chunkDevice = new ChunkDevice(device, startPos, _uploadChunkSize);
        chunkDevice->open(QIODevice::ReadOnly);

        QNetworkReply *reply = networkManager()->put(req, chunkDevice);
        chunkDevice->setParent(reply);
        _chunkedUploads.insert(reply, qMakePair(device, endPos));
        ereply->setNetworkReply(reply);
        _connections.append(QObject::connect(reply, &QNetworkReply::uploadProgress, UploadProgressFunctor(this, reply)));
    }
};

#undef CHECK_AND_SET_URL_PATH_IMPL
#undef CHECK_AND_SET_PATH_WITH_ID
#undef CHECK_AND_SET_PATH

#endif // ENGINIOCLIENT_P_H
