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

#include "enginioabstractobjectfactory.h"
#include "enginioclient.h"
#include "enginioreply.h"
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

class FactoryUnit
{
public:
    EnginioAbstractObjectFactory *factory;
    int id;

    static int nextId;
};


struct ENGINIOCLIENT_EXPORT EnginioString
{
    static const QString pageSize;
    static const QString limit;
    static const QString offset;
    static const QString include;
    static const QString query;
    static const QString message;
    static const QString results;
    static const QString _synced;
    static const QString objectType;
    static const QString id;
    static const QString username;
    static const QString password;
    static const QString sessionToken;
    static const QString authIdentity;
    static const QString files;
    static const QString search;
    static const QString session;
    static const QString users;
    static const QString usergroups;
    static const QString object;
    static const QString url;
    static const QString access;
    static const QString sort;
    static const QString count;
    static const QString targetFileProperty;
    static const QString members;
    static const QString propertyName;
    static const QString apiEnginIo;
};

class ENGINIOCLIENT_EXPORT EnginioClientPrivate
{
    enum PathOptions { Default, IncludeIdInPath = 1};

    template<class T>
    static QString getPath(const T &object, int operation, PathOptions flags = Default)
    {
        QString result;
        result.reserve(96);
        result.append(QStringLiteral("/v1/"));

        // FIXME warn about invalid paths

        switch (operation) {
        case ObjectOperation: {
            QString objectType = object[EnginioString::objectType].toString();
            if (objectType.isEmpty())
                return QString();

            result.append(objectType.replace('.', '/'));
            break;
        }
        case ObjectAclOperation:
        {
            QString objectType = object[EnginioString::objectType].toString();
            if (objectType.isEmpty())
                return QString();

            result.append(objectType.replace('.', '/'));
            QString id = object[EnginioString::id].toString();
            if (id.isEmpty())
                return QString();
            result.append('/');
            result.append(id);
            result.append('/');
            result.append(EnginioString::access);
            return result;
        }
        case AuthenticationOperation:
            result.append(EnginioString::authIdentity);
            break;
        case FileOperation:
            result.append(EnginioString::files);
            break;
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
            if (id.isEmpty())
                return QString();
            result.append(EnginioString::usergroups);
            result.append('/');
            result.append(id);
            result.append('/');
            result.append(EnginioString::members);
            return result;
        }
        }

        if (flags & IncludeIdInPath) {
            QString id = object[EnginioString::id].toString();
            if (id.isEmpty())
                return QString();
            result.append('/');
            result.append(id);
        }

        return result;
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
            EnginioReply *ereply = d->_replyReplyMap.take(nreply);

            if (!ereply)
                return;

            EnginioClient *q = static_cast<EnginioClient*>(d->q_ptr);

            if (nreply->error() != QNetworkReply::NoError) {
                d->_downloads.remove(nreply);
                emit q->error(ereply);
            }

            // resolve the download url
            if (d->_downloads.contains(nreply)) {
                QString propertyName = d->_downloads.value(nreply);
                d->_downloads.remove(nreply);
                QString id = ereply->data()[EnginioString::results].toArray().first().toObject()[propertyName].toObject()[EnginioString::id].toString();
                QUrl url(d->m_apiUrl);
                url.setPath(getPath(QJsonObject(), FileOperation) + QLatin1Char('/') + id + QStringLiteral("/download_url"));
                //url.setQuery("variant=original");
                QNetworkRequest req(d->_request);
                req.setUrl(url);
                QNetworkReply *reply = d->q_ptr->networkManager()->get(req);
                ereply->setNetworkReply(reply);
                return;
            }

            ereply->emitFinished();
            q->finished(ereply);
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
            _identity->prepareSessionToken(_enginio);
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

public:
    enum Operation {
        // Do not forget to keep in sync with EnginioClient::Operation!
        ObjectOperation = EnginioClient::ObjectOperation,
        ObjectAclOperation = EnginioClient::ObjectAclOperation,
        UserOperation = EnginioClient::UserOperation,
        UsergroupOperation = EnginioClient::UsergroupOperation,

        // private
        UsergroupMemberOperation,
        AuthenticationOperation,
        SessionOperation,
        SearchOperation,
        FileOperation
    };

    Q_ENUMS(Operation)

    EnginioClientPrivate(EnginioClient *client = 0);
    virtual ~EnginioClientPrivate();

    int addFactory(EnginioAbstractObjectFactory *factory);
    void removeFactory(int factoryId);

    EnginioClient *q_ptr;
    QString m_backendId;
    QString m_backendSecret;
    EnginioIdentity *_identity;
    QVarLengthArray<QMetaObject::Connection, 3> _identityConnections;
    QUrl m_apiUrl;
    QNetworkAccessManager *m_networkManager;
    QMetaObject::Connection _networkManagerConnection;
    QList<FactoryUnit*> m_factories;
    QNetworkRequest _request;
    QMap<QNetworkReply*, EnginioReply*> _replyReplyMap;
    QMap<QNetworkReply*, QString> _downloads;

    QByteArray sessionToken() const
    {
        return _request.rawHeader(QByteArrayLiteral("Enginio-Backend-Session"));
    }

    void setSessionToken(const QByteArray &sessionToken)
    {
        _request.setRawHeader(QByteArrayLiteral("Enginio-Backend-Session"), sessionToken);

        emit q_ptr->sessionTokenChanged(sessionToken);
        if (sessionToken.isEmpty())
            emit q_ptr->sessionTerminated();
        else
            emit q_ptr->sessionAuthenticated();
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
            // invalidate old token
            q_ptr->setSessionToken(QByteArray());
            return;
        }
        CallPrepareSessionToken callPrepareSessionToken(this, identity);
        if (!isInitialized()) {
            _identityConnections.append(QObject::connect(q_ptr, &EnginioClient::clientInitialized, callPrepareSessionToken));
        } else
            identity->prepareSessionToken(this);
        _identityConnections.append(QObject::connect(identity, &EnginioIdentity::dataChanged, callPrepareSessionToken));
        _identityConnections.append(QObject::connect(identity, &EnginioIdentity::destroyed, IdentityInstanceDestroyed(this)));
        emit q_ptr->identityChanged(identity);
    }

    QNetworkReply *identify(const QJsonObject &object)
    {
        QUrl url(m_apiUrl);
        url.setPath(getPath(object, AuthenticationOperation));

        QNetworkRequest req(_request);
        req.setUrl(url);
        QByteArray data(QJsonDocument(object).toJson(QJsonDocument::Compact));
        return q_ptr->networkManager()->post(req, data);
    }

    template<class T>
    QNetworkReply *update(const ObjectAdaptor<T> &object, const EnginioClient::Operation operation)
    {
        QUrl url(m_apiUrl);
        url.setPath(getPath(object, operation, IncludeIdInPath));

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
        return q_ptr->networkManager()->put(req, data);
    }

    template<class T>
    QNetworkReply *remove(const ObjectAdaptor<T> &object, const EnginioClient::Operation operation)
    {
        QUrl url(m_apiUrl);
        url.setPath(getPath(object, operation, IncludeIdInPath));

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
            QNetworkReply *reply = q_ptr->networkManager()->sendCustomRequest(req, QByteArrayLiteral("DELETE"), buffer);
            buffer->setParent(reply);
            return reply;
        }
        return q_ptr->networkManager()->deleteResource(req);
#else
        QByteArray data = o.toJson();
        return q_ptr->networkManager()->deleteResource(req, data);
#endif
    }

    template<class T>
    QNetworkReply *create(const ObjectAdaptor<T> &object, const EnginioClient::Operation operation)
    {
        QUrl url(m_apiUrl);
        url.setPath(getPath(object, operation));

        QNetworkRequest req(_request);
        req.setUrl(url);

        QByteArray data = object.toJson();
        return q_ptr->networkManager()->post(req, data);
    }

    template<class T>
    QNetworkReply *query(const ObjectAdaptor<T> &object, const Operation operation)
    {
        QUrl url(m_apiUrl);
        url.setPath(getPath(object, operation));

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
            ArrayAdaptor<T> objectTypes = object[QStringLiteral("objectTypes")].toArray();
            if (search.isComposedType()) {
                for (typename ArrayAdaptor<T>::const_iterator i = objectTypes.constBegin(); i != objectTypes.constEnd(); ++i) {
                    urlQuery.addQueryItem(QStringLiteral("objectTypes[]"), (*i).toString());
                }
                urlQuery.addQueryItem(EnginioString::search,
                    QString::fromUtf8(search.toJson()));

                // FIXME: Think about proper error handling for wrong user input.
            } else qWarning("!! Fulltext Search: parameter(s) missing !!");
        } else
        if (object[EnginioString::query].isComposedType()) { // TODO docs are inconsistent on that
            urlQuery.addQueryItem(QStringLiteral("q"),
                QString::fromUtf8(object[EnginioString::query].toJson()));
        }
        url.setQuery(urlQuery);

        QNetworkRequest req(_request);
        req.setUrl(url);

        return q_ptr->networkManager()->get(req);
    }

    bool isInitialized() const
    {
        return !m_backendId.isEmpty() && !m_backendSecret.isEmpty();
    }

    QNetworkReply *downloadFile(const QJsonObject &object)
    {
        QString id = object[EnginioString::id].toString();
        QString objectType = object[EnginioString::objectType].toString();
        QJsonObject obj;
        obj = QJsonDocument::fromJson(QByteArrayLiteral(
                    "{\"include\": {\"file\": {}},"
                     "\"objectType\": \"") + objectType.toUtf8() + "\","
                     "\"query\": {\"id\": \"" + id.toUtf8() + "\"}}").object();

        QNetworkReply *reply = query<QJsonObject>(obj, ObjectOperation);
        _downloads.insert(reply, object[EnginioString::propertyName].toString());
        return reply;
    }

    template<class T>
    QNetworkReply *uploadFile(const ObjectAdaptor<T> &object, const QUrl &fileUrl)
    {
        Q_ASSERT_X(fileUrl.scheme().isEmpty() || fileUrl.isLocalFile(), "", "Upload must be local file.");
        QString path = fileUrl.isLocalFile() ? fileUrl.toLocalFile() : fileUrl.path();

        QFile *file = new QFile(path);
        file->open(QFile::ReadOnly);
        Q_ASSERT(file->isOpen());

        QString fileName = file->fileName();
        Q_ASSERT(!fileName.isEmpty());
        QMimeDatabase mimeDb;
        QString mimeType = mimeDb.mimeTypeForFile(fileUrl.toLocalFile()).name();
        return upload(object, fileName, file, mimeType);
    }

    template<class T>
    QNetworkReply *upload(const ObjectAdaptor<T> &object, const QString &fileName, QIODevice *device, const QString &mimeType)
    {
        QUrl apiUrl = m_apiUrl;
        apiUrl.setPath(getPath(QJsonObject(), FileOperation));

        QNetworkRequest req(_request);
        // FIXME: must NOT have the json type as content type,
        // otherwise enginio thinks we want to download
        // unsetting it here is ugly
        req.setHeader(QNetworkRequest::ContentTypeHeader, QString());
        req.setUrl(apiUrl);

        QHttpMultiPart *multiPart = createHttpMultiPart(fileName, device, mimeType, object.toJson());
        QNetworkReply *reply = q_ptr->networkManager()->post(req, multiPart);
        multiPart->setParent(reply);
        device->setParent(multiPart);
        return reply;
    }

    QNetworkAccessManager *networkManager()
    {
        return m_networkManager;
    }

    void assignNetworkManager();
    static QNetworkAccessManager *prepareNetworkManagerInThread();

    bool isSignalConnected(const QMetaMethod &signal) const
    {
        return q_ptr->isSignalConnected(signal);
    }

private:
    /* Create a multi part upload:
     * That means the JSON metadata and the actual file get sent in one http-post.
     * The associatedObject has to be a valid object type on the server.
     * If it does not contain an id, it needs to be manually associated later or will get garbage collected eventually.
     */
    QHttpMultiPart *createHttpMultiPart(const QString &fileName, QIODevice *data, const QString &mimeType, const QByteArray &associatedObject)
    {
        Q_ASSERT(!fileName.isEmpty());

        // check file/chunk size
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        data->setParent(multiPart);

        QHttpPart objectPart;
        objectPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                             QStringLiteral("form-data; name=\"object\""));

        objectPart.setBody(associatedObject);
        multiPart->append(objectPart);

        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QStringLiteral("form-data; name=\"file\"; filename=\"%1\"").arg(fileName));
        filePart.setBodyDevice(data);
        multiPart->append(filePart);
        return multiPart;
    }
};

#endif // ENGINIOCLIENT_P_H
