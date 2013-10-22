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

#include "enginioidentity.h"
#include "enginiobasicauthentication.h"
#include "enginiooauth2authentication.h"
#include "enginioclient_p.h"
#include "enginioreply.h"

#include <QtCore/qjsonobject.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qstring.h>
#include <QtNetwork/qnetworkreply.h>

QT_BEGIN_NAMESPACE

/*!
  \class EnginioIdentity
  \inmodule enginio-qt
  \ingroup enginio-client
  \brief Represents a user that is authenticated with the backend
  This class is an abstract base class for the different authentication methods and is never used directly.
*/

/*!
  \fn EnginioIdentity::dataChanged()
  \internal
*/

/*!
    Constructs a new EnginioIdentity with \a parent as QObject parent.
*/
EnginioIdentity::EnginioIdentity(QObject *parent) :
    QObject(parent)
{
}

class EnginioUserPassAuthenticationPrivate;
struct DisconnectConnection
{
    EnginioUserPassAuthenticationPrivate *auth;

    DisconnectConnection(EnginioUserPassAuthenticationPrivate *authentication)
        : auth(authentication)
    {}

    inline void operator ()() const;
};

class EnginioUserPassAuthenticationPrivate
{
    template<typename T>
    class SessionSetterFunctor
    {
        EnginioClientPrivate *_enginio;
        QNetworkReply *_reply;
        EnginioUserPassAuthenticationPrivate *_auth;
    public:
        SessionSetterFunctor(EnginioClientPrivate *enginio, QNetworkReply *reply, EnginioUserPassAuthenticationPrivate *auth)
            : _enginio(enginio)
            , _reply(reply)
            , _auth(auth)
        {}
        void operator ()()
        {
            // TODO it creates a finished EnginioReplyBase, there will be no finished signal
            EnginioReplyBase *ereply = _enginio->createReply(_reply);
            if (_reply->error() != QNetworkReply::NoError) {
                emit _enginio->emitSessionAuthenticationError(ereply);
                // TODO does ereply leak? Yes potentially. We need to think about the ownership
            } else {
                _auth->thisAs<T>()->proccessToken(_enginio, ereply);
                _enginio->emitSessionAuthenticated(ereply);
            }
        }
    };

    QPointer<QNetworkReply> _reply;
    QMetaObject::Connection _replyFinished;
    QMetaObject::Connection _enginioDestroyed;

public:
    QString _user;
    QString _pass;

    ~EnginioUserPassAuthenticationPrivate();

    template<class Derived>
    Derived *thisAs() { return static_cast<Derived*>(this); }

    void cleanupConnections()
    {
        if (_reply) {
            QObject::disconnect(_replyFinished);
            QObject::disconnect(_enginioDestroyed);
            QObject::connect(_reply.data(), &QNetworkReply::finished, _reply.data(), &QNetworkReply::deleteLater);
            _reply = 0;
        }
    }

    template<typename Derived>
    void prepareSessionToken(EnginioClientPrivate *enginio)
    {
        cleanupConnections();

        _reply = thisAs<Derived>()->makeRequest(enginio);
        enginio->setAuthenticationState(EnginioClientBase::Authenticating);
        _replyFinished = QObject::connect(_reply.data(), &QNetworkReply::finished, SessionSetterFunctor<Derived>(enginio, _reply.data(), this));
        _enginioDestroyed = QObject::connect(enginio->q_ptr, &EnginioClient::destroyed, DisconnectConnection(this));
    }

    template<typename Derived>
    void removeSessionToken(EnginioClientPrivate *enginio)
    {
        cleanupConnections();
        thisAs<Derived>()->cleanupClient(enginio);
        _reply = 0;
        enginio->emitSessionTerminated();
    }
};

struct EnginioBasicAuthenticationPrivate: public EnginioUserPassAuthenticationPrivate
{
    QNetworkReply *makeRequest(EnginioClientPrivate *enginio)
    {
        QJsonObject object;
        object[EnginioString::username] = _user;
        object[EnginioString::password] = _pass;

        QUrl url(enginio->_serviceUrl);
        url.setPath(EnginioString::v1_auth_identity);

        QNetworkRequest req = enginio->prepareRequest(url);
        QByteArray data(QJsonDocument(object).toJson(QJsonDocument::Compact));

        return enginio->networkManager()->post(req, data);
    }

    void proccessToken(EnginioClientPrivate *enginio, EnginioReplyBase *reply)
    {
        QByteArray sessionToken;
        if (reply) {
            enginio->_identityToken = reply->data();
            sessionToken = enginio->_identityToken[EnginioString::sessionToken].toString().toLatin1();
        }

        enginio->_request.setRawHeader(EnginioString::Enginio_Backend_Session, sessionToken);
    }

    void cleanupClient(EnginioClientPrivate *enginio)
    {
        enginio->_request.setRawHeader(EnginioString::Enginio_Backend_Session, QByteArray());
    }
};

/*!
  \class EnginioBasicAuthentication
  \inmodule enginio-qt
  \ingroup enginio-client
  \brief Represents a user that is authenticated directly by the backend.

  This class can authenticate a user by verifying the user's login and password.
  The user has to exist in the backend already.

  To authenticate an instance of EnginioClient called \a client such code may be used:
  \code
    EnginioBasicAuthentication identity;
    identity.setUser(_user);
    identity.setPassword(_user);

    client.setIdentity(&identity);
  \endcode

  Setting the identity will trigger an asynchronous request, resulting in EnginioClient::authenticationState()
  changing.

  \sa EnginioClientBase::authenticationState() EnginioClientBase::identity() EnginioClient::sessionAuthenticated()
  \sa EnginioClient::sessionAuthenticationError() EnginioClient::sessionTerminated()

*/

/*!
  \property EnginioBasicAuthentication::user
  This property contains the user name used for authentication.
*/

/*!
  \property EnginioBasicAuthentication::password
  This property contains the password used for authentication.
*/

/*!
  Constructs a EnginioBasicAuthentication instance with \a parent as QObject parent.
*/
EnginioBasicAuthentication::EnginioBasicAuthentication(QObject *parent)
    : EnginioIdentity(parent)
    , d_ptr(new EnginioBasicAuthenticationPrivate())
{
    connect(this, &EnginioBasicAuthentication::userChanged, this, &EnginioIdentity::dataChanged);
    connect(this, &EnginioBasicAuthentication::passwordChanged, this, &EnginioIdentity::dataChanged);
}

/*!
  Destructs this EnginioBasicAuthentication instance.
*/
EnginioBasicAuthentication::~EnginioBasicAuthentication()
{
    emit aboutToDestroy();
}

QString EnginioBasicAuthentication::user() const
{
    return d_ptr->_user;
}

void EnginioBasicAuthentication::setUser(const QString &user)
{
    if (d_ptr->_user == user)
        return;
    d_ptr->_user = user;
    emit userChanged(user);
}

QString EnginioBasicAuthentication::password() const
{
    return d_ptr->_pass;
}

void EnginioBasicAuthentication::setPassword(const QString &password)
{
    if (d_ptr->_pass == password)
        return;
    d_ptr->_pass = password;
    emit passwordChanged(password);
}

/*!
  \internal
*/
void EnginioBasicAuthentication::prepareSessionToken(EnginioClientPrivate *enginio)
{
    Q_ASSERT(enginio);
    Q_ASSERT(enginio->identity());

    d_ptr->prepareSessionToken<EnginioBasicAuthenticationPrivate>(enginio);
}

/*!
  \internal
*/
void EnginioBasicAuthentication::removeSessionToken(EnginioClientPrivate *enginio)
{
    Q_ASSERT(enginio);
    Q_ASSERT(enginio->identity());
    d_ptr->removeSessionToken<EnginioBasicAuthenticationPrivate>(enginio);
}

struct EnginioOAuth2AuthenticationPrivate: public EnginioUserPassAuthenticationPrivate
{
    QNetworkReply *makeRequest(EnginioClientPrivate *enginio)
    {
        QByteArray data;
        {
            QUrlQuery urlQuery;
            urlQuery.addQueryItem(EnginioString::grant_type, EnginioString::password);
            urlQuery.addQueryItem(EnginioString::username, _user);
            urlQuery.addQueryItem(EnginioString::password, _pass);
            data = urlQuery.query().toUtf8();
        }

        QUrl url(enginio->_serviceUrl);
        url.setPath(EnginioString::v1_auth_oauth2_tokens);

        QNetworkRequest request(enginio->prepareRequest(url));
        request.setHeader(QNetworkRequest::ContentTypeHeader, EnginioString::Application_x_www_form_urlencoded);
        request.setRawHeader(EnginioString::Accept, EnginioString::Application_json);
        // request.setRawHeader("Enginio-Backend-Secret", QByteArray()); TODO do we need to remove it?

        return enginio->networkManager()->post(request, data);;
    }

    void proccessToken(EnginioClientPrivate *enginio, EnginioReplyBase *ereply)
    {
        QByteArray header;
        header = EnginioString::Bearer_ + ereply->data()[EnginioString::access_token].toString().toUtf8();
        enginio->_request.setRawHeader(EnginioString::Authorization, header);
    }

    void cleanupClient(EnginioClientPrivate *enginio)
    {
        enginio->_request.setRawHeader(EnginioString::Authorization, QByteArray());
    }
};

/*!
  Constructs a EnginioPasswordOAuth2 instance with \a parent as QObject parent.
*/
EnginioOAuth2Authentication::EnginioOAuth2Authentication(QObject *parent)
    : EnginioIdentity(parent)
    , d_ptr(new EnginioOAuth2AuthenticationPrivate())
{
    connect(this, &EnginioOAuth2Authentication::userChanged, this, &EnginioIdentity::dataChanged);
    connect(this, &EnginioOAuth2Authentication::passwordChanged, this, &EnginioIdentity::dataChanged);
}

/*!
  Destructs this EnginioPasswordOAuth2 instance.
*/
EnginioOAuth2Authentication::~EnginioOAuth2Authentication()
{
    emit aboutToDestroy();
}

/*!
  \property EnginioOAuth2Authentication::user
  This property contains the user name used for authentication.
*/

QString EnginioOAuth2Authentication::user() const
{
    return d_ptr->_user;
}

void EnginioOAuth2Authentication::setUser(const QString &user)
{
    if (d_ptr->_user == user)
        return;
    d_ptr->_user = user;
    emit userChanged(user);
}

/*!
  \property EnginioOAuth2Authentication::password
  This property contains the password used for authentication.
*/

QString EnginioOAuth2Authentication::password() const
{
    return d_ptr->_pass;
}

void EnginioOAuth2Authentication::setPassword(const QString &password)
{
    if (d_ptr->_pass == password)
        return;
    d_ptr->_pass = password;
    emit passwordChanged(password);
}

/*!
  \internal
*/
void EnginioOAuth2Authentication::prepareSessionToken(EnginioClientPrivate *enginio)
{
    Q_ASSERT(enginio);
    Q_ASSERT(enginio->identity());

    d_ptr->prepareSessionToken<EnginioOAuth2AuthenticationPrivate>(enginio);
}

/*!
  \internal
*/
void EnginioOAuth2Authentication::removeSessionToken(EnginioClientPrivate *enginio)
{
    Q_ASSERT(enginio);
    Q_ASSERT(enginio->identity());
    d_ptr->removeSessionToken<EnginioOAuth2AuthenticationPrivate>(enginio);
}

void DisconnectConnection::operator ()() const
{
    auth->cleanupConnections();
}

EnginioUserPassAuthenticationPrivate::~EnginioUserPassAuthenticationPrivate()
{}

QT_END_NAMESPACE
