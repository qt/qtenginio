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
#include <QtCore/qurlquery.h>


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

        if (area == EnginioClient::ObjectsArea) {
            QString objectType = object[QStringLiteral("objectType")].toString();
            if (objectType.isEmpty())
                return QString();

            result.append(objectType.replace('.', '/'));
        }

        if (area == EnginioClient::UsersArea) {
            result.append(QStringLiteral("users"));
        }

        if (area == EnginioClient::AuthenticationArea) {
            result.append(QStringLiteral("auth/identity"));
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

    struct ReplyFinishedFunctor {
        EnginioClientPrivate *d;
        void operator ()(QNetworkReply *nreply)
        {
            Q_ASSERT(d);
            EnginioClient *q = static_cast<EnginioClient*>(d->q_func());
            EnginioReply *ereply = d->_replyReplyMap.take(nreply);

            // FIXME
            if (!ereply)
                return;
//            Q_ASSERT(ereply);

            q->finished(ereply);
            ereply->finished();
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
            struct CallPrepareSessionToken
            {
                EnginioClientPrivate *enginio;
                EnginioIdentity *identity;
                void operator ()()
                {
                    identity->prepareSessionToken(enginio);
                }
            };
            _identityConnection = QObject::connect(q_ptr, &EnginioClient::clientInitialized, CallPrepareSessionToken{this, identity});
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
};

#endif // ENGINIOCLIENT_P_H
