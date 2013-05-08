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

#include "enginioidentity.h"
#include "enginioclient_p.h"

#include <QtCore/qjsonobject.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qstring.h>
#include <QtNetwork/qnetworkreply.h>

EnginioIdentity::EnginioIdentity(QObject *parent) :
    QObject(parent)
{
}

struct EnginioAuthenticationPrivate
{
    class IdentifyFunctor {
        struct SessionSetterFunctor {
            EnginioClientPrivate *enginio;
            QNetworkReply *reply;
            void operator ()()
            {
                QByteArray data(reply->readAll());
                QJsonObject message(QJsonDocument::fromJson(data).object());
                QByteArray token = message[QStringLiteral("sessionToken")].toString().toLatin1();
                enginio->setSessionToken(token);
                reply->deleteLater();
            }
        };
    public:
        EnginioClientPrivate *enginio;
        EnginioAuthenticationPrivate *authentication;
        void operator ()()
        {
            Q_ASSERT(enginio);
            Q_ASSERT(enginio->isInitialized());
            Q_ASSERT(enginio->identity());

            QJsonObject data;
            data[QStringLiteral("username")] = authentication->_user;
            data[QStringLiteral("password")] = authentication->_pass;
            QNetworkReply *reply = enginio->identify(data);
            QObject::connect(reply, &QNetworkReply::finished, SessionSetterFunctor{enginio, reply});
        }
    };

    EnginioAuthenticationPrivate(EnginioAuthentication *q)
        : q_ptr(q)
    {}
    EnginioAuthentication *q_ptr;
    QString _user;
    QString _pass;
};

EnginioAuthentication::EnginioAuthentication(QObject *parent)
    : EnginioIdentity(parent)
    , d_ptr(new EnginioAuthenticationPrivate(this))
{
}

QString EnginioAuthentication::user() const
{
    return d_ptr->_user;
}

void EnginioAuthentication::setUser(const QString &user)
{
    if (d_ptr->_user == user)
        return;
    d_ptr->_user = user;
    emit userChanged(user);
}

QString EnginioAuthentication::password() const
{
    return d_ptr->_pass;
}

void EnginioAuthentication::setPassword(const QString &password)
{
    if (d_ptr->_pass == password)
        return;
    d_ptr->_pass = password;
    emit userChanged(password);
}

void EnginioAuthentication::prepareSessionToken(EnginioClientPrivate *enginio)
{
    EnginioAuthenticationPrivate::IdentifyFunctor ident{enginio, d_ptr};
    ident();
}

