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

#ifndef ENGINIOCLIENTBASE_H
#define ENGINIOCLIENTBASE_H

#include <Enginio/enginioclient_global.h>
#include <QObject>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtypeinfo.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;
class EnginioIdentity;
class EnginioClientPrivate;

class ENGINIOCLIENT_EXPORT EnginioClientBase : public QObject
{
    Q_OBJECT
public:
    enum AuthenticationState {
        NotAuthenticated,
        Authenticating,
        Authenticated,
        AuthenticationFailure
    };
    Q_ENUMS(AuthenticationState)

    enum Operation {
        // Do not forget to keep in sync with EnginioClientPrivate::Operation!
        ObjectOperation,
        ObjectAclOperation,
        UserOperation,
        UsergroupOperation,
        UsergroupMembersOperation,
        FileOperation
    };
    Q_ENUMS(Operation)

    explicit EnginioClientBase(QObject *parent, EnginioClientPrivate *d);
    ~EnginioClientBase();

    Q_PROPERTY(QByteArray backendId READ backendId WRITE setBackendId NOTIFY backendIdChanged FINAL)
    Q_PROPERTY(QByteArray backendSecret READ backendSecret WRITE setBackendSecret NOTIFY backendSecretChanged FINAL)
    Q_PROPERTY(QUrl serviceUrl READ serviceUrl WRITE setServiceUrl NOTIFY serviceUrlChanged FINAL)
    Q_PROPERTY(EnginioIdentity *identity READ identity WRITE setIdentity NOTIFY identityChanged FINAL)
    Q_PROPERTY(AuthenticationState authenticationState READ authenticationState NOTIFY authenticationStateChanged FINAL)

    QByteArray backendId() const Q_REQUIRED_RESULT;
    void setBackendId(const QByteArray &backendId);
    QByteArray backendSecret() const Q_REQUIRED_RESULT;
    void setBackendSecret(const QByteArray &backendSecret);
    EnginioIdentity *identity() const Q_REQUIRED_RESULT;
    void setIdentity(EnginioIdentity *identity);
    AuthenticationState authenticationState() const Q_REQUIRED_RESULT;

    QUrl serviceUrl() const Q_REQUIRED_RESULT;
    void setServiceUrl(const QUrl &serviceUrl);
    QNetworkAccessManager *networkManager() const Q_REQUIRED_RESULT;

    bool finishDelayedReplies();

Q_SIGNALS:
    void backendIdChanged(const QByteArray &backendId);
    void backendSecretChanged(const QByteArray &backendSecret);
    void serviceUrlChanged(const QUrl& url);
    void authenticationStateChanged(const AuthenticationState state);
    void identityChanged(const EnginioIdentity *identity);

protected:
    QScopedPointer<EnginioClientPrivate> d_ptr;
    Q_DECLARE_PRIVATE(EnginioClient)
private:
    Q_DISABLE_COPY(EnginioClientBase)
};

Q_DECLARE_TYPEINFO(EnginioClientBase::Operation, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(EnginioClientBase::AuthenticationState, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(EnginioClientBase::Operation)
Q_DECLARE_METATYPE(EnginioClientBase::AuthenticationState)

#endif // ENGINIOCLIENTBASE_H
