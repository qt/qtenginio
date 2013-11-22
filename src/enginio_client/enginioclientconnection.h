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

#ifndef ENGINIOCLIENTCONNECTION_H
#define ENGINIOCLIENTCONNECTION_H

#include <Enginio/enginioclient_global.h>
#include <QObject>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtypeinfo.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;
class EnginioIdentity;
class EnginioClientConnectionPrivate;

class ENGINIOCLIENT_EXPORT EnginioClientConnection : public QObject
{
    Q_OBJECT
    Q_ENUMS(AuthenticationState)
    Q_ENUMS(Operation)

    Q_PROPERTY(QByteArray backendId READ backendId WRITE setBackendId NOTIFY backendIdChanged FINAL)
    Q_PROPERTY(QUrl serviceUrl READ serviceUrl WRITE setServiceUrl NOTIFY serviceUrlChanged FINAL)
    Q_PROPERTY(EnginioIdentity *identity READ identity WRITE setIdentity NOTIFY identityChanged FINAL)
    Q_PROPERTY(AuthenticationState authenticationState READ authenticationState NOTIFY authenticationStateChanged FINAL)

public:
    enum AuthenticationState {
        NotAuthenticated,
        Authenticating,
        Authenticated,
        AuthenticationFailure
    };

    enum Operation {
        // Do not forget to keep in sync with EnginioClientPrivate::Operation!
        ObjectOperation,
        AccessControlOperation,
        UserOperation,
        UsergroupOperation,
        UsergroupMembersOperation,
        FileOperation
    };

    ~EnginioClientConnection();


    QByteArray backendId() const Q_REQUIRED_RESULT;
    void setBackendId(const QByteArray &backendId);
    EnginioIdentity *identity() const Q_REQUIRED_RESULT;
    void setIdentity(EnginioIdentity *identity);
    AuthenticationState authenticationState() const Q_REQUIRED_RESULT;

    QUrl serviceUrl() const Q_REQUIRED_RESULT;
    void setServiceUrl(const QUrl &serviceUrl);
    QNetworkAccessManager *networkManager() const Q_REQUIRED_RESULT;

    bool finishDelayedReplies();

Q_SIGNALS:
    void backendIdChanged(const QByteArray &backendId);
    void serviceUrlChanged(const QUrl& url);
    void authenticationStateChanged(AuthenticationState state);
    void identityChanged(EnginioIdentity *identity);

protected:
    explicit EnginioClientConnection(EnginioClientConnectionPrivate &dd, QObject *parent);

private:
    Q_DECLARE_PRIVATE(EnginioClientConnection)
    Q_DISABLE_COPY(EnginioClientConnection)
};

Q_DECLARE_TYPEINFO(EnginioClientConnection::Operation, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(EnginioClientConnection::AuthenticationState, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(EnginioClientConnection::Operation)
Q_DECLARE_METATYPE(EnginioClientConnection::AuthenticationState)

#endif // ENGINIOCLIENTCONNECTION_H
