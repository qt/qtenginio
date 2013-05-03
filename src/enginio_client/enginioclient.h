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

class EnginioAbstractObject;
class EnginioAbstractObjectFactory;
class EnginioClientPrivate;
class QNetworkAccessManager;
class QUrl;

class QNetworkReply;
class QSslError;

class ENGINIOCLIENT_EXPORT EnginioClient : public QObject
{
    Q_OBJECT
public:
    explicit EnginioClient(const QString &backendId,
                           const QString &backendSecret,
                           QObject *parent = 0);
    ~EnginioClient();

    QString backendId() const;
    void setBackendId(const QString &backendId);
    QString backendSecret() const;
    void setBackendSecret(const QString &backendSecret);
    QUrl apiUrl() const;
    void setApiUrl(const QUrl &apiUrl);
    QNetworkAccessManager * networkManager();
    void setNetworkManager(QNetworkAccessManager *networkManager);
    QString sessionToken() const;
    void setSessionToken(const QString &sessionToken);

    int registerObjectFactory(EnginioAbstractObjectFactory *factory);
    void unregisterObjectFactory(int factoryId);
    EnginioAbstractObject * createObject(const QString &type,
                                         const QString &id = QString()) const;

signals:
    void sessionAuthenticated() const;
    void sessionTerminated() const;

private slots:
    void ignoreSslErrors(QNetworkReply* reply, const QList<QSslError> &errors);

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

#endif // ENGINIOCLIENT_H
