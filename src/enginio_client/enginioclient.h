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
        UserOperation,
        UsergroupOperation
    };
    Q_ENUMS(Operation)

    explicit EnginioClient(const QString &backendId,
                           const QString &backendSecret,
                           QObject *parent = 0);
    explicit EnginioClient(QObject *parent = 0);
    ~EnginioClient();

    Q_PROPERTY(QString backendId READ backendId WRITE setBackendId NOTIFY backendIdChanged)
    Q_PROPERTY(QString backendSecret READ backendSecret WRITE setBackendSecret NOTIFY backendSecretChanged)
    Q_PROPERTY(QUrl apiUrl READ apiUrl WRITE setApiUrl NOTIFY apiUrlChanged)
    Q_PROPERTY(bool initialized READ isInitialized NOTIFY clientInitialized)
    Q_PROPERTY(QByteArray sessionToken READ sessionToken WRITE setSessionToken NOTIFY sessionTokenChanged)
    Q_PROPERTY(EnginioIdentity *identity READ identity WRITE setIdentity NOTIFY identityChanged)

    QString backendId() const;
    void setBackendId(const QString &backendId);
    QString backendSecret() const;
    void setBackendSecret(const QString &backendSecret);
    EnginioIdentity *identity() const;
    void setIdentity(EnginioIdentity *identity);


    QUrl apiUrl() const;
    void setApiUrl(const QUrl &apiUrl);
    QNetworkAccessManager *networkManager();
    QByteArray sessionToken() const;
    void setSessionToken(const QByteArray &sessionToken);

    bool isInitialized() const;

    int registerObjectFactory(EnginioAbstractObjectFactory *factory);
    void unregisterObjectFactory(int factoryId);
    EnginioAbstractObject *createObject(const QString &type,
                                         const QString &id = QString()) const;

    Q_INVOKABLE EnginioReply *search(const QJsonObject &query);
    Q_INVOKABLE EnginioReply *query(const QJsonObject &query, const Operation operation = ObjectOperation);
    Q_INVOKABLE EnginioReply *create(const QJsonObject &object, const Operation operation = ObjectOperation);
    Q_INVOKABLE EnginioReply *update(const QJsonObject &object, const Operation operation = ObjectOperation);
    Q_INVOKABLE EnginioReply *remove(const QJsonObject &object, const Operation operation = ObjectOperation);

    EnginioReply *uploadFile(const QJsonObject &associatedObject, const QUrl &file);
    EnginioReply *downloadFile(const QJsonObject &object);

signals:
    void sessionAuthenticated() const;
    void sessionTerminated() const;
    void clientInitialized() const;
    void backendIdChanged(const QString &backendId);
    void backendSecretChanged(const QString &backendSecret);
    void apiUrlChanged();
    void sessionTokenChanged();
    void identityChanged(const EnginioIdentity *identity);
    void finished(EnginioReply *reply);

private slots:
    void ignoreSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

protected:
    EnginioClientPrivate *d_ptr;
    EnginioClient(const QString &backendId,
                  const QString &backendSecret,
                  EnginioClientPrivate &dd,
                  QObject *parent = 0);

private:
    Q_DECLARE_PRIVATE(EnginioClient)
    Q_DISABLE_COPY(EnginioClient)
};

Q_DECLARE_TYPEINFO(EnginioClient::Operation, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(EnginioClient::Operation);

#endif // ENGINIOCLIENT_H
