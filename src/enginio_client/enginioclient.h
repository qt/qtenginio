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

#ifndef ENGINIOCLIENT_H
#define ENGINIOCLIENT_H

#include "enginioclient_global.h"
#include <QObject>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtypeinfo.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qurl.h>

class EnginioAbstractObject;
class EnginioAbstractObjectFactory;
class EnginioClientPrivate;
class QNetworkAccessManager;

class QNetworkReply;
class QSslError;
class EnginioReply;
class EnginioIdentity;

class ENGINIOCLIENT_EXPORT EnginioClient : public QObject
{
    Q_OBJECT
public:
    enum Operation {
        // Do not forget to keep in sync with EnginioClientPrivate::Operation!
        ObjectOperation,
        ObjectAclOperation,
        UserOperation,
        UsergroupOperation,
        UsergroupMembersOperation
    };
    Q_ENUMS(Operation)

    explicit EnginioClient(const QString &backendId,
                           const QString &backendSecret,
                           QObject *parent = 0);
    explicit EnginioClient(QObject *parent = 0);
    ~EnginioClient();

    Q_PROPERTY(QString backendId READ backendId WRITE setBackendId NOTIFY backendIdChanged REVISION 1)
    Q_PROPERTY(QString backendSecret READ backendSecret WRITE setBackendSecret NOTIFY backendSecretChanged REVISION 1)
    Q_PROPERTY(QUrl apiUrl READ apiUrl WRITE setApiUrl NOTIFY apiUrlChanged REVISION 1)
    Q_PROPERTY(bool initialized READ isInitialized NOTIFY clientInitialized REVISION 1)
    Q_PROPERTY(EnginioIdentity *identity READ identity WRITE setIdentity NOTIFY identityChanged REVISION 1)
    Q_PROPERTY(QJsonObject identityToken READ identityToken NOTIFY identityTokenChanged REVISION 1)

    QString backendId() const;
    void setBackendId(const QString &backendId);
    QString backendSecret() const;
    void setBackendSecret(const QString &backendSecret);
    EnginioIdentity *identity() const;
    void setIdentity(EnginioIdentity *identity);
    QJsonObject identityToken() const;

    QUrl apiUrl() const;
    void setApiUrl(const QUrl &apiUrl);
    QNetworkAccessManager *networkManager();

    bool isInitialized() const;

    Q_INVOKABLE Q_REVISION(1) EnginioReply *search(const QJsonObject &query);
    Q_INVOKABLE Q_REVISION(1) EnginioReply *query(const QJsonObject &query, const Operation operation = ObjectOperation);
    Q_INVOKABLE Q_REVISION(1) EnginioReply *create(const QJsonObject &object, const Operation operation = ObjectOperation);
    Q_INVOKABLE Q_REVISION(1) EnginioReply *update(const QJsonObject &object, const Operation operation = ObjectOperation);
    Q_INVOKABLE Q_REVISION(1) EnginioReply *remove(const QJsonObject &object, const Operation operation = ObjectOperation);

    Q_INVOKABLE Q_REVISION(1) EnginioReply *uploadFile(const QJsonObject &associatedObject, const QUrl &file);
    Q_INVOKABLE Q_REVISION(1) EnginioReply *downloadFile(const QJsonObject &object);

signals:
    Q_REVISION(1) void sessionAuthenticated() const;
    Q_REVISION(1) void sessionAuthenticationError(EnginioReply *reply) const;
    Q_REVISION(1) void sessionTerminated() const;
    Q_REVISION(1) void clientInitialized() const;
    Q_REVISION(1) void backendIdChanged(const QString &backendId);
    Q_REVISION(1) void backendSecretChanged(const QString &backendSecret);
    Q_REVISION(1) void apiUrlChanged(const QUrl& url);
    Q_REVISION(1) void identityTokenChanged(const QJsonObject &token);
    Q_REVISION(1) void identityChanged(const EnginioIdentity *identity);
    Q_REVISION(1) void finished(EnginioReply *reply);
    Q_REVISION(1) void error(EnginioReply *reply);

private slots:
    void ignoreSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

protected:
    QScopedPointer<EnginioClientPrivate> d_ptr;

    EnginioClient(QObject *parent, EnginioClientPrivate *d);
    Q_DECLARE_PRIVATE(EnginioClient)
private:
    Q_DISABLE_COPY(EnginioClient)
    friend class EnginioReply;
};

Q_DECLARE_TYPEINFO(EnginioClient::Operation, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(EnginioClient::Operation);

#endif // ENGINIOCLIENT_H
