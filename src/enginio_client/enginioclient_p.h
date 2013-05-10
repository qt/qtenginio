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

class FactoryUnit
{
public:
    EnginioAbstractObjectFactory *factory;
    int id;

    static int nextId;
};


class EnginioClientPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(EnginioClient)

    enum PathOptions { Default, IncludeIdInPath = 1};
    static QString getPath(const QJsonObject &object, const EnginioClient::Area area, PathOptions flags = Default)
    {
        QString result;
        result.reserve(32);
        result.append(QStringLiteral("/v1/"));

        switch (area) {
        case EnginioClient::ObjectsArea: {
            QString objectType = object[QStringLiteral("objectType")].toString();
            if (objectType.isEmpty())
                return QString();

            result.append(objectType.replace('.', '/'));
            break;
        }
        case EnginioClient::AuthenticationArea:
            result.append(QStringLiteral("auth/identity"));
            break;
        case EnginioClient::FileArea:
            result.append(QStringLiteral("files"));
            break;
        case EnginioClient::FulltextSearchArea:
            result.append(QStringLiteral("search"));
            break;
        case EnginioClient::SessionArea:
            result.append(QStringLiteral("session"));
            break;
        case EnginioClient::UsersArea:
            result.append(QStringLiteral("users"));
            break;
        case EnginioClient::UsergroupsArea:
            result.append(QStringLiteral("usergroups"));
            break;
        case EnginioClient::UsergroupMembersArea:
            // FIXME usergroups/{id}/members
            result.append(QStringLiteral("usergroups"));
            break;
        }

        if (flags & IncludeIdInPath) {
            QString id = object[QStringLiteral("id")].toString();
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
        {}
        void operator ()(QNetworkReply *nreply)
        {
            Q_ASSERT(d);
            EnginioClient *q = static_cast<EnginioClient*>(d->q_func());
            EnginioReply *ereply = d->_replyReplyMap.take(nreply);

            // FIXME
            if (!ereply)
                return;
//            Q_ASSERT(ereply);


            if (d->_uploads.contains(nreply)) {
                d->_uploads.remove(nreply);
                QJsonObject object;
                object[QStringLiteral("id")] = ereply->data()[QStringLiteral("object")].toObject()[QStringLiteral("id")].toString();
                object[QStringLiteral("objectType")] = ereply->data()[QStringLiteral("object")].toObject()[QStringLiteral("objectType")].toString();

                QJsonObject fileRef;
                fileRef.insert(QStringLiteral("id"), ereply->data()[QStringLiteral("id")].toString());
                fileRef.insert(QStringLiteral("objectType"), QStringLiteral("files"));

                // FIXME: do not hard-code file here
                object[QStringLiteral("file")] = fileRef;

                d->_replyReplyMap.insert(d->update(object, EnginioClient::ObjectsArea), ereply);
                return;
            } else if (d->_downloads.contains(nreply)) {
                QUrl url = d->m_apiUrl;
                url.setPath(ereply->data()["results"].toArray().first().toObject()["file"].toObject()["url"].toString());
                qDebug() << "Download URL: " << url;
            }

            q->finished(ereply);
            ereply->finished();
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
public:
    EnginioClientPrivate(EnginioClient *client = 0);
    virtual ~EnginioClientPrivate();

    int addFactory(EnginioAbstractObjectFactory *factory);
    void removeFactory(int factoryId);

    EnginioClient *q_ptr;
    QString m_backendId;
    QString m_backendSecret;
    EnginioIdentity *_identity;
    QMetaObject::Connection _identityConnection;
    QUrl m_apiUrl;
    QPointer<QNetworkAccessManager> m_networkManager;
    bool m_deleteNetworkManager;
    QList<FactoryUnit*> m_factories;
    QNetworkRequest _request;
    QMap<QNetworkReply*, EnginioReply*> _replyReplyMap;
    QSet<QNetworkReply*> _uploads;
    QSet<QNetworkReply*> _downloads;

    QByteArray sessionToken() const
    {
        return _request.rawHeader(QByteArrayLiteral("Enginio-Backend-Session"));
    }

    void setSessionToken(const QByteArray &sessionToken)
    {
        _request.setRawHeader(QByteArrayLiteral("Enginio-Backend-Session"), sessionToken);

        emit q_ptr->sessionTokenChanged();
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
        QObject::disconnect(_identityConnection);
        if (!(_identity = identity)) {
            // invalidate old token
            q_ptr->setSessionToken(QByteArray());
            return;
        }
        if (!isInitialized()) {
            _identityConnection = QObject::connect(q_ptr, &EnginioClient::clientInitialized, CallPrepareSessionToken(this, identity));
        } else
            identity->prepareSessionToken(this);
        emit q_ptr->identityChanged(identity);
    }

    QNetworkReply *identify(const QJsonObject &object)
    {
        QUrl url(m_apiUrl);
        url.setPath(getPath(object, EnginioClient::AuthenticationArea));

        QNetworkRequest req(_request);
        req.setUrl(url);
        QByteArray data(QJsonDocument(object).toJson(QJsonDocument::Compact));
        return q_ptr->networkManager()->post(req, data);
    }

    QNetworkReply *update(const QJsonObject &object, const EnginioClient::Area area)
    {
        QUrl url(m_apiUrl);
        url.setPath(getPath(object, area, IncludeIdInPath));

        QNetworkRequest req(_request);
        req.setUrl(url);

        QByteArray data = QJsonDocument(object).toJson(QJsonDocument::Compact);
        return q_ptr->networkManager()->put(req, data);
    }

    QNetworkReply *remove(const QJsonObject &object, const EnginioClient::Area area)
    {
        QUrl url(m_apiUrl);
        url.setPath(getPath(object, area, IncludeIdInPath));

        QNetworkRequest req(_request);
        req.setUrl(url);

        return q_ptr->networkManager()->deleteResource(req);
    }

    QNetworkReply *create(const QJsonObject &object, const EnginioClient::Area area)
    {
        QUrl url(m_apiUrl);
        url.setPath(getPath(object, area));

        QNetworkRequest req(_request);
        req.setUrl(url);

        QByteArray data = QJsonDocument(object).toJson(QJsonDocument::Compact);
        return q_ptr->networkManager()->post(req, data);
    }

    QNetworkReply *query(const QJsonObject &object, const EnginioClient::Area area)
    {
        QUrl url(m_apiUrl);
        url.setPath(getPath(object, area));

        // TODO add all params here
        QUrlQuery urlQuery;
        if (int limit = object[QStringLiteral("limit")].toDouble()) {
            urlQuery.addQueryItem(QStringLiteral("limit"), QString::number(limit));
        }
        QJsonValue include = object[QStringLiteral("include")];
        if (include.isObject()) {
            urlQuery.addQueryItem(QStringLiteral("include"),
                QString::fromUtf8(QJsonDocument(include.toObject()).toJson(QJsonDocument::Compact)));
        }
        if (object[QStringLiteral("query")].isObject()) { // TODO docs are inconsistent on that
            urlQuery.addQueryItem(QStringLiteral("q"),
                QString::fromUtf8(QJsonDocument(object[QStringLiteral("query")].toObject()).toJson(QJsonDocument::Compact)));
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
        QString id = object["id"].toString();
        QString objectType = object["objectType"].toString();
        QJsonObject obj;
        obj = QJsonDocument::fromJson(
                    "{\"include\": {\"file\": {}},"
                     "\"objectType\": \"" + objectType.toUtf8() + "\","
                     "\"query\": {\"id\": \"" + id.toUtf8() + "\"}}").object();

        QNetworkReply *reply = query(obj, EnginioClient::ObjectsArea);
        _downloads.insert(reply);
        return reply;
    }

    QNetworkReply *uploadFile(const QJsonObject &associatedObject, const QUrl &fileUrl)
    {
        Q_ASSERT_X(fileUrl.isLocalFile(), "", "Upload must be local file.");
        QFile *file = new QFile(fileUrl.toLocalFile());
        if (!file->open(QFile::ReadOnly))
            // FIXME: this is not allowed
            return 0;
        Q_ASSERT(file->isOpen());

        QString fileName = file->fileName();
        Q_ASSERT(!fileName.isEmpty());
        QMimeDatabase mimeDb;
        QString mimeType = mimeDb.mimeTypeForFile(fileUrl.toLocalFile()).name();
        return upload(associatedObject, fileName, file, mimeType);
    }

    QNetworkReply *upload(const QJsonObject &associatedObject, const QString &fileName, QIODevice *device, const QString &mimeType)
    {
        QUrl apiUrl = m_apiUrl;
        apiUrl.setPath(getPath(QJsonObject(), EnginioClient::FileArea));

        QNetworkRequest req(_request);
        // FIXME: must NOT have the json type as content type,
        // otherwise enginio thinks we want to download
        // unsetting it here is ugly
        req.setHeader(QNetworkRequest::ContentTypeHeader, QString());
        req.setUrl(apiUrl);

        QJsonObject obj;
        obj[QStringLiteral("object")] = associatedObject;
        QByteArray object = QJsonDocument(obj).toJson();
        QHttpMultiPart *multiPart = createHttpMultiPart(fileName, device, mimeType, object);
        QNetworkReply *reply = q_ptr->networkManager()->post(req, multiPart);
        multiPart->setParent(reply);
        device->setParent(multiPart);
        _uploads.insert(reply);
        return reply;
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
