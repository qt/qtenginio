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

#include <Enginio/enginioclient.h>
#include <Enginio/enginioreply.h>
#include <QtCore/qjsonarray.h>
#include <QSignalSpy>

#include "common.h"

namespace EnginioTests {

EnginioBackendManager::EnginioBackendManager(QObject *parent)
    : QObject(parent)
    , _email(qgetenv("ENGINIO_EMAIL_ADDRESS"))
    , _password(qgetenv("ENGINIO_LOGIN_PASSWORD"))
    , _url(EnginioTests::TESTAPP_URL)
{
    if (_email.isEmpty() || _password.isEmpty()) {
        qDebug("Needed environment variables ENGINIO_EMAIL_ADDRESS, ENGINIO_LOGIN_PASSWORD are not set. Backend setup failed!");
        return;
    }
    QObject::connect(&_client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    QObject::connect(&_client, SIGNAL(finished(EnginioReply *)), this, SLOT(finished(EnginioReply *)));
}

EnginioBackendManager::~EnginioBackendManager()
{}

void EnginioBackendManager::finished(EnginioReply *reply)
{
    Q_ASSERT(reply);
    _responseData = reply->data();
}

void EnginioBackendManager::error(EnginioReply *reply)
{
    Q_ASSERT(reply);
    qDebug() << "\n\n### ERROR";
    qDebug() << reply->errorString();
    reply->dumpDebugInfo();
    qDebug() << "\n###\n";
}

bool EnginioBackendManager::synchronousRequest(const QByteArray &httpOperation, const QJsonObject &data)
{
    QSignalSpy finishedSpy(&_client, SIGNAL(finished(EnginioReply *)));
    QSignalSpy errorSpy(&_client, SIGNAL(error(EnginioReply *)));
    _client.customRequest(_url, httpOperation, data);
    return finishedSpy.wait(10000) && !errorSpy.count();
}

QString EnginioBackendManager::authenticate()
{
    qDebug("\tAuthenticating with Enginio server.");
    QJsonObject obj;
    QJsonObject credentials;
    credentials["email"] = _email;
    credentials["password"] = _password;
    QJsonObject headers;
    headers["Accept"] = QStringLiteral("application/json");
    obj["payload"] = credentials;
    obj["headers"] = headers;
    _url.setPath(QStringLiteral("/v1/account/auth/identity"));

    // Authenticate developer
    if (!synchronousRequest(QByteArrayLiteral("POST"), obj))
        return QString();

    return _responseData["sessionToken"].toString();
}

bool EnginioBackendManager::createBackend(const QString& backendName)
{
    qDebug("\t## Creating backend: %s", backendName.toUtf8().data());
    QString sessionToken = authenticate();
    if (sessionToken.isEmpty()) {
        qDebug("ERROR: Session authentication failed!");
        return false;
    }

    // Get all backends
    QJsonObject obj;
    QJsonObject headers;
    headers["Accept"] = QStringLiteral("application/json");
    headers["Enginio-Backend-Session"] = sessionToken;
    _url.setPath("/v1/account/apps");
    obj["headers"] = headers;

    if (!synchronousRequest(QByteArrayLiteral("GET"), obj))
        return false;

    QJsonArray results = _responseData["results"].toArray();
    qDebug() << "\t" << results.count() << "backends found.";
    foreach (const QJsonValue& value, results) {
        if (value.toObject()["name"].toString() == backendName) {
            removeBackend(backendName);
            break;
        }
    }

    QJsonObject backend;
    backend["name"] = backendName;
    obj["payload"] = backend;

    if (!synchronousRequest(QByteArrayLiteral("POST"), obj))
        return false;

    return true;
}

bool EnginioBackendManager::removeBackend(const QString &backendName)
{
    qDebug("\t## Deleting backend: %s", backendName.toUtf8().data());
     QString sessionToken = authenticate();
     if (sessionToken.isEmpty()) {
        qDebug("ERROR: Session authentication failed!");
        return false;
    }

    // Get all backends
    QJsonObject obj;
    QJsonObject headers;
    QString appsPath = QStringLiteral("/v1/account/apps");
    headers["Accept"] = QStringLiteral("application/json");
    headers["Enginio-Backend-Session"] = sessionToken;
    _url.setPath(appsPath);
    obj["headers"] = headers;

    if (!synchronousRequest(QByteArrayLiteral("GET"), obj))
        return false;

    QString backendId;
    QJsonArray results = _responseData["results"].toArray();
    foreach (const QJsonValue& value, results) {
        QJsonObject data = value.toObject();
        if (data["name"].toString() == backendName) {
            backendId = data["id"].toString();
            break;
        }
    }

    if (backendId.isEmpty())
        return false;

    appsPath.append("/").append(backendId);
    _url.setPath(appsPath);
    if (!synchronousRequest(QByteArrayLiteral("DELETE"), obj))
        return false;

    return true;
}

}
