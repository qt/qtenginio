/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
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

#include <QtTest/QtTest>
#include <QtCore/qobject.h>
#include <QtCore/qthread.h>

#include <Enginio/enginioclient.h>
#include <Enginio/enginioreply.h>
#include <Enginio/enginioidentity.h>

#include "../common/common.h"

#define CHECK_NO_ERROR(response) \
    QVERIFY(!response->isError()); \
    QCOMPARE(response->errorType(), EnginioReply::NoError);\
    QCOMPARE(response->networkError(), QNetworkReply::NoError);\
    QVERIFY(response->backendStatus() >= 200 && response->backendStatus() < 300);

class tst_EnginioClient: public QObject
{
    Q_OBJECT

public slots:
    void error(EnginioReply *reply) {
        qDebug() << "\n\n### ERROR";
        qDebug() << reply->errorString();
        reply->dumpDebugInfo();
        qDebug() << "\n###\n";
    }

private slots:
    void initTestCase();
    void init();
    void customRequest();
    void query_todos();
    void query_todos_filter();
    void query_todos_limit();
    void query_todos_count();
    void query_todos_sort();
    void users_crud();
    void query_users();
    void query_users_filter();
    void query_users_sort();
    void query_usersgroup_crud();
    void query_usersgroup_limit();
    void query_usersgroup_count();
    void query_usersgroup_sort();
    void query_usersgroupmembers_limit();
    void query_usersgroupmembers_count();
    void query_usersgroupmembers_sort();
    void search();
    void create_todos();
    void update_todos();
    void update_invalidId();
    void remove_todos();
    void identity();
    void identity_invalid();
    void identity_afterLogout();
    void backendFakeReply();
    void acl();
    void sharingNetworkManager();

private:
    QString usergroupId(EnginioClient *client)
    {
        static QString id;
        static QString backendId = client->backendId();
        Q_ASSERT(backendId == client->backendId()); // client can be changed but we want to use the same backend id
        if (id.isEmpty()) {
            QJsonObject obj;
            obj["limit"] = 1;
            const EnginioReply *reqId = client->query(obj, EnginioClient::UsergroupOperation);
            Q_ASSERT(reqId);
            const int __step = 50;
            const int __timeoutValue = 5000;
            for (int __i = 0; __i < __timeoutValue && (reqId->data().isEmpty()); __i+=__step) {
                QTest::qWait(__step);
            }
            id = reqId->data()["results"].toArray().first().toObject()["id"].toString();
            Q_ASSERT(!id.isEmpty());
        }
        return id;
    }

    void populateForSearchIfNeeded() {
        EnginioClient client;
        QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
        client.setserviceUrl(EnginioTests::TESTAPP_URL);

        QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));
        int iterations = 10;

        {
            QJsonObject obj;
            obj["objectType"] = QString::fromUtf8("objects.CustomObject");
            EnginioReply *reply = client.query(obj);
            QTRY_COMPARE(spy.count(), 1);
            QCOMPARE(spyError.count(), 0);
            QVERIFY2(!reply->data().isEmpty(), "Empty data in EnginioReply.");
            QVERIFY2(!reply->data()["results"].isUndefined(), "Undefined results array in EnginioReply data.");
            bool create = reply->data()["results"].toArray().isEmpty();
            spy.clear();

            if (create) {
                for (int i = 0; i < iterations; ++i)
                {
                    obj["intValue"] = i;
                    obj["stringValue"] = QString::fromUtf8("Query object");
                    client.create(obj);
                }

                QTRY_COMPARE(spy.count(), iterations);
                QCOMPARE(spyError.count(), 0);
            }

            spy.clear();
        }
        {
            QJsonObject obj;
            obj["objectType"] = QString::fromUtf8("objects.SelfLinkedObject");
            EnginioReply *reply = client.query(obj);
            QTRY_COMPARE(spy.count(), 1);
            QCOMPARE(spyError.count(), 0);
            QVERIFY2(!reply->data().isEmpty(), "Empty data in EnginioReply.");
            QVERIFY2(!reply->data()["results"].isUndefined(), "Undefined results array in EnginioReply data.");
            bool create = reply->data()["results"].toArray().isEmpty();
            spy.clear();

            if (create) {
                for (int i = 0; i < iterations; ++i)
                {
                    obj["intValue"] = i;
                    obj["stringValue"] = QString::fromUtf8("object test");
                    client.create(obj);
                }

                QTRY_COMPARE(spy.count(), iterations);
                QCOMPARE(spyError.count(), 0);
            }
        }
    }
};

void tst_EnginioClient::initTestCase()
{
    // Create some users to be used in later tests
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    int spyCount = spy.count();

    for (int i = 0; i < 5; ++i) {
        QJsonObject query;
        query["username"] = QStringLiteral("logintest") + (i ? QString::number(i) : "");
        obj["query"] = query;
        client.query(obj, EnginioClient::UserOperation);
        ++spyCount;
    }

    QTRY_COMPARE(spy.count(), spyCount);
    QCOMPARE(spyError.count(), 0);

    for (int i = 0; i < 5; ++i) {
        EnginioReply *reply = spy[i][0].value<EnginioReply*>();
        QVERIFY(reply);
        QVERIFY(!reply->data().isEmpty());
        QVERIFY(!reply->data()["query"].isUndefined());
        obj = reply->data()["query"].toObject();
        QVERIFY(!obj.isEmpty());
        obj = obj["query"].toObject();
        QVERIFY(!obj.isEmpty());
        QString identity = obj["username"].toString();
        QVERIFY(!identity.isEmpty());
        QVERIFY(!reply->data()["results"].isUndefined());
        QJsonArray data = reply->data()["results"].toArray();
        if (!data.count()) {
            QJsonObject query;
            query["username"] = identity;
            query["password"] = identity;
            client.create(query, EnginioClient::UserOperation);
            ++spyCount;
            qDebug() << "Creating " << query;
        }
    }

    QTRY_COMPARE(spy.count(), spyCount);
    QCOMPARE(spyError.count(), 0);


    // Create user groups for later tests if the backend does not have them yet.
    spy.clear();
    spyCount = 0;

    for (int i = 0; i < 5; ++i) {
        QJsonObject query;
        query["name"] = "usergroup" + (i ? QString::number(i) : "");
        obj["query"] = query;
        client.query(obj, EnginioClient::UsergroupOperation);
        ++spyCount;
    }

    QTRY_COMPARE(spy.count(), spyCount);
    QCOMPARE(spyError.count(), 0);

    for (int i = 0; i < 5; ++i) {
        EnginioReply *reply = spy[i][0].value<EnginioReply*>();
        QVERIFY(reply);
        QVERIFY(!reply->data().isEmpty());
        QVERIFY(!reply->data()["query"].isUndefined());
        obj = reply->data()["query"].toObject();
        QVERIFY(!obj.isEmpty());
        obj = obj["query"].toObject();
        QVERIFY(!obj.isEmpty());
        QString groupName = obj["name"].toString();
        QVERIFY(!groupName.isEmpty());
        QVERIFY(!reply->data()["results"].isUndefined());
        QJsonArray data = reply->data()["results"].toArray();
        if (!data.count()) {
            QJsonObject query;
            query["name"] = groupName;
            client.create(query, EnginioClient::UsergroupOperation);
            ++spyCount;
            qDebug() << "Creating " << query;
        }
    }

    QTRY_COMPARE(spy.count(), spyCount);
    QCOMPARE(spyError.count(), 0);
}

void tst_EnginioClient::init()
{
    if (EnginioTests::TESTAPP_ID.isEmpty() || EnginioTests::TESTAPP_SECRET.isEmpty() || EnginioTests::TESTAPP_URL.isEmpty())
        QFAIL("Needed environment variables ENGINIO_BACKEND_ID, ENGINIO_BACKEND_SECRET, ENGINIO_API_URL are not set!");
}

void tst_EnginioClient::customRequest()
{
    QString backendName = QStringLiteral("EnginioClient") + QString::number(QDateTime::currentMSecsSinceEpoch());
    EnginioTests::EnginioBackendManager backendManager(this);

    QVERIFY(backendManager.createBackend(backendName));
    QJsonObject schema;
    schema["name"] = QStringLiteral("places");
    QJsonArray array;
    QJsonObject title;
    title["name"] = QStringLiteral("title");
    title["type"] = QStringLiteral("string");
    title["indexed"] = false;
    QJsonObject photo;
    photo["name"] = QStringLiteral("photo");
    photo["type"] = QStringLiteral("ref");
    photo["objectType"] = QStringLiteral("files");
    array.append(title);
    array.append(photo);
    schema["properties"] = array;
    QVERIFY(backendManager.createObjectType(backendName, QStringLiteral("development"), schema));
    QVERIFY(backendManager.removeBackend(backendName));
}

void tst_EnginioClient::query_todos()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply *)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    const EnginioReply* reqId = client.query(obj);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QVERIFY(!response->data().isEmpty());
    QVERIFY(!response->data()["results"].isUndefined());
}

void tst_EnginioClient::query_todos_filter()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    obj["query"] = QJsonDocument::fromJson("{\"completed\": true}").object();
    const EnginioReply* reqId = client.query(obj);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(!data["results"].isUndefined());
    QJsonArray array = data["results"].toArray();
    foreach (QJsonValue value, array)
        QVERIFY(value.toObject()["completed"].toBool());
}

void tst_EnginioClient::query_todos_limit()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    obj["limit"] = 1;
    const EnginioReply* reqId = client.query(obj);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(data["results"].isArray());
    QCOMPARE(data["results"].toArray().count(), 1);
}

void tst_EnginioClient::query_todos_count()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    obj["count"] = true;
    const EnginioReply* reqId = client.query(obj);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(data.contains("count"));
}

void tst_EnginioClient::query_todos_sort()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    obj["limit"] = 400;
    obj["sort"] = QJsonDocument::fromJson(
                QByteArrayLiteral("[{\"sortBy\": \"title\", \"direction\": \"asc\"}, {\"sortBy\": \"createdAt\", \"direction\": \"asc\"}]")).array();
    const EnginioReply* reqId = client.query(obj);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    if (reqId->networkError())
        qDebug() << reqId->data();
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QJsonArray results = data["results"].toArray();
    QVERIFY(results.count());
    QString previousTitle, currentTitle;
    QString previousCreatedAt, currentCreatedAt;
    for (int i = 0; i < results.count(); ++i) {
        currentTitle = results[i].toObject()["title"].toString();
        currentCreatedAt = results[i].toObject()["createdAt"].toString();
        QVERIFY(currentTitle > previousTitle
                || (currentTitle == previousTitle && currentCreatedAt >= previousCreatedAt));
        previousTitle = currentTitle;
        previousCreatedAt = currentCreatedAt;
    }
}

void tst_EnginioClient::query_users()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    const EnginioReply* reqId = client.query(obj, EnginioClient::UserOperation);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(!data["results"].isUndefined());
    QVERIFY(data["results"].toArray().count());
}

void tst_EnginioClient::query_users_sort()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    {
        QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));
        QJsonObject obj;
        obj["sort"] = QJsonDocument::fromJson(QByteArrayLiteral("[{\"sortBy\": \"createdAt\", \"direction\": \"asc\"}]")).array();
        const EnginioReply *reqId = client.query(obj, EnginioClient::UserOperation);
        QVERIFY(reqId);

        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);

        const EnginioReply *response = spy[0][0].value<EnginioReply*>();
        QCOMPARE(response, reqId);
        CHECK_NO_ERROR(response);
        QJsonObject data = response->data();
        QVERIFY(!data.isEmpty());
        QVERIFY(!data["results"].isUndefined());
        QJsonArray results = data["results"].toArray();
        QVERIFY(results.count());
        QString previous, current;
        for (int i = 0; i < results.count(); ++i) {
            current = results[i].toObject()["createdAt"].toString();
            QVERIFY(current >= previous);
            previous = current;
        }
    }
    {
        QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));
        QJsonObject obj;
        obj["sort"] = QJsonDocument::fromJson(QByteArrayLiteral("[{\"sortBy\": \"createdAt\", \"direction\": \"desc\"}]")).array();
        const EnginioReply *reqId = client.query(obj, EnginioClient::UserOperation);
        QVERIFY(reqId);

        QTRY_COMPARE(spy.count(), 1);
        if (reqId->networkError())
            qDebug() << reqId->data();
        QCOMPARE(spyError.count(), 0);

        const EnginioReply *response = spy[0][0].value<EnginioReply*>();
        QCOMPARE(response, reqId);
        CHECK_NO_ERROR(response);
        QJsonObject data = response->data();
        QVERIFY(!data.isEmpty());
        QVERIFY(!data["results"].isUndefined());
        QJsonArray results = data["results"].toArray();
        QVERIFY(results.count());
        QString previous, current;
        for (int i = 0; i < results.count(); ++i) {
            current = results[i].toObject()["createdAt"].toString();
            QVERIFY(current <= previous || previous.isEmpty());
            previous = current;
        }
    }
}

void tst_EnginioClient::query_usersgroup_crud()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QString groupName = QStringLiteral("users");
    QString tmpGroupName = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString description = QStringLiteral("Temporary Usergroup");
    QJsonObject data;
    data["name"] = tmpGroupName;

    qDebug() << data;

    EnginioReply *reply = client.create(data, EnginioClient::UsergroupOperation);
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);
    QVERIFY(reply);
    QVERIFY(!reply->data().isEmpty());
    QCOMPARE(reply->data()["name"].toString(), tmpGroupName);

    QVERIFY(!reply->data()["id"].isUndefined());
    spy.clear();

    QString id = reply->data()["id"].toString();
    QJsonObject query;
    query["query"] = data;

    reply = client.query(query, EnginioClient::UsergroupOperation);
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);
    QVERIFY(reply);
    QVERIFY(!reply->data().isEmpty());
    QVERIFY(!reply->data()["results"].isUndefined());
    QJsonArray results = reply->data()["results"].toArray();
    QCOMPARE(results.count(), 1);
    QCOMPARE(results[0].toObject()["id"].toString(), id);
    spy.clear();


    data["id"] = id;
    data["description"] = description;
    reply = client.update(data, EnginioClient::UsergroupOperation);
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);
    QVERIFY(reply);
    QVERIFY(!reply->data().isEmpty());
    QVERIFY(!reply->data()["name"].isUndefined());
    QCOMPARE(reply->data()["name"].toString(), tmpGroupName);
    QCOMPARE(reply->data()["description"].toString(), description);
    spy.clear();

    data = QJsonObject();
    data["id"] = id;
    reply = client.remove(data, EnginioClient::UsergroupOperation);
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);
    QVERIFY(reply);
}

void tst_EnginioClient::query_usersgroup_limit()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    obj["limit"] = 1;
    const EnginioReply *reqId = client.query(obj, EnginioClient::UsergroupOperation);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(!data["results"].isUndefined());
    QVERIFY(data["results"].toArray().count());
}

void tst_EnginioClient::query_usersgroup_count()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    obj["count"] = 1;
    const EnginioReply *reqId = client.query(obj, EnginioClient::UsergroupOperation);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(data.contains("count"));
}

void tst_EnginioClient::query_usersgroup_sort()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    obj["sort"] = QJsonDocument::fromJson(QByteArrayLiteral("[{\"sortBy\": \"createdAt\", \"direction\": \"asc\"}]")).array();
    const EnginioReply *reqId = client.query(obj, EnginioClient::UsergroupOperation);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(!data["results"].isUndefined());
    QJsonArray results = data["results"].toArray();
    QVERIFY(results.count());
    QString previous, current;
    for (int i = 0; i < results.count(); ++i) {
        current = results[i].toObject()["createdAt"].toString();
        QVERIFY(current >= previous);
        previous = current;
    }
}

void tst_EnginioClient::query_usersgroupmembers_limit()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QString id = usergroupId(&client);
    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    obj["limit"] = 1;
    obj["id"] = id;
    const EnginioReply *reqId = client.query(obj, EnginioClient::UsergroupMembersOperation);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(!data["results"].isUndefined());
    QVERIFY(data["results"].toArray().count() <= 1);
}

void tst_EnginioClient::query_usersgroupmembers_count()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QString id = usergroupId(&client);
    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    obj["count"] = 1;
    obj["id"] = id;
    const EnginioReply *reqId = client.query(obj, EnginioClient::UsergroupMembersOperation);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(data.contains("count"));
}

void tst_EnginioClient::query_usersgroupmembers_sort()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QString id = usergroupId(&client);
    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    obj["id"] = id;
    obj["sort"] = QJsonDocument::fromJson(QByteArrayLiteral("[{\"sortBy\": \"createdAt\", \"direction\": \"desc\"}]")).array();
    const EnginioReply *reqId = client.query(obj, EnginioClient::UsergroupMembersOperation);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(!data["results"].isUndefined());
    QJsonArray results = data["results"].toArray();
    QString previous, current;
    for (int i = 0; i < results.count(); ++i) {
        current = results[i].toObject()["createdAt"].toString();
        QVERIFY(current >= previous);
        previous = current;
    }
}
void tst_EnginioClient::query_users_filter()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj = QJsonDocument::fromJson("{\"query\":{\"username\": \"john\"}}").object();
    QVERIFY(!obj.isEmpty());
    const EnginioReply* reqId = client.query(obj, EnginioClient::UserOperation);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(!data["results"].isUndefined());
    QVERIFY(!data["results"].toArray().count());
}

void tst_EnginioClient::search()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    populateForSearchIfNeeded();

    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    int resultCount1 = 0;
    {
        QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
        QJsonObject searchQuery = QJsonDocument::fromJson(
                    "{\"objectTypes\": [\"objects.CustomObject\"],"
                    "\"search\": {\"phrase\": \"Query\"}}").object();

        QVERIFY(!searchQuery.isEmpty());
        const EnginioReply* reqId = client.search(searchQuery);
        QVERIFY(reqId);


        if (!spy.wait())
            reqId->dumpDebugInfo();

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);

        const EnginioReply *response = spy[0][0].value<EnginioReply*>();

        QCOMPARE(response, reqId);
        CHECK_NO_ERROR(response);
        QJsonObject data = response->data();
        QVERIFY(!data.isEmpty());
        QVERIFY(!data["results"].isUndefined());
        resultCount1 = data["results"].toArray().count();
        QVERIFY(resultCount1);
        qDebug() << resultCount1 << "results on objects.CustomObject with phrase \"Query\".";
    }
    {
        QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
        QJsonObject searchQuery = QJsonDocument::fromJson(
                    "{\"objectTypes\": [\"objects.CustomObject\", \"objects.SelfLinkedObject\"],"
                    "\"search\": {\"phrase\": \"object OR test\", \"properties\": [\"stringValue\"]}}").object();

        QVERIFY(!searchQuery.isEmpty());
        const EnginioReply* reqId = client.search(searchQuery);
        QVERIFY(reqId);

        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);

        const EnginioReply *response = spy[0][0].value<EnginioReply*>();

        QCOMPARE(response, reqId);
        CHECK_NO_ERROR(response);
        QJsonObject data = response->data();
        QVERIFY(!data.isEmpty());
        QVERIFY(!data["results"].isUndefined());
        int resultCount2 = data["results"].toArray().count();
        QVERIFY(resultCount2 > resultCount1);
        qDebug() << resultCount2 << " results on objects.CustomObject and objects.SelfLinkedObject with phrase \"object OR test\".";
    }
}

void tst_EnginioClient::create_todos()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    obj["title"] = QString::fromUtf8("test title");
    obj["completed"] = false;
    const EnginioReply* reqId = client.create(obj);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QCOMPARE(data["completed"], obj["completed"]);
    QCOMPARE(data["title"], obj["title"]);
    QCOMPARE(data["objectType"], obj["objectType"]);
}

void tst_EnginioClient::users_crud()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QString name = QUuid::createUuid().toString();
    QString pass = QString::fromUtf8("Metaphysics");
    QString id;
    {
        // CREATE
        int spyCount = spy.count();
        QJsonObject obj;
        obj["username"] = name;
        obj["password"] = pass;

        const EnginioReply* reqId = client.create(obj, EnginioClient::UserOperation);
        QVERIFY(reqId);

        QTRY_COMPARE(spy.count(), spyCount + 1);
        QCOMPARE(spyError.count(), 0);

        const EnginioReply *response = spy[0][0].value<EnginioReply*>();
        QJsonObject data = response->data();
        QCOMPARE(response, reqId);
        CHECK_NO_ERROR(response);
        QVERIFY(!data.isEmpty());
        QCOMPARE(data["username"], obj["username"]);
        QVERIFY(!data["id"].toString().isEmpty());
        id = data["id"].toString();
    }
    {
        // READ
        int spyCount = spy.count();
        QJsonObject query;
        query["username"] = name;
        QJsonObject obj;
        obj["query"] = query;
        const EnginioReply* reqId = client.query(obj, EnginioClient::UserOperation);
        QVERIFY(reqId);
        QTRY_COMPARE(spy.count(), spyCount + 1);
        QCOMPARE(spyError.count(), 0);
        CHECK_NO_ERROR(reqId);
        QJsonArray data = reqId->data()["results"].toArray();
        QCOMPARE(data.count(), 1);
        QCOMPARE(data[0].toObject()["id"].toString(), id);
        QCOMPARE(data[0].toObject()["username"].toString(), name);
    }
    {
        // UPDATE
        int spyCount = spy.count();
        QJsonObject obj;
        obj["password"] = pass + "pass";
        obj["id"] = id;
        const EnginioReply* reqId = client.update(obj, EnginioClient::UserOperation);
        QVERIFY(reqId);
        QTRY_COMPARE(spy.count(), spyCount + 1);
        QCOMPARE(spyError.count(), 0);
        CHECK_NO_ERROR(reqId);
        QJsonObject data = reqId->data();
        QCOMPARE(data["id"].toString(), id);
        QCOMPARE(data["username"].toString(), name);
    }
    {
        int spyCount = spy.count();
        // DELETE it
        QJsonObject obj;
        obj["id"] = id;
        const EnginioReply* reqId = client.remove(obj, EnginioClient::UserOperation);
        QVERIFY(reqId);

        QTRY_COMPARE(spy.count(), spyCount + 1);
        QCOMPARE(spyError.count(), 0);
    }
}

void tst_EnginioClient::update_todos()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject query;
    query["objectType"] = QString::fromUtf8("objects.todos");
    query["limit"] = 1;
    const EnginioReply* reqId = client.query(query);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QJsonObject object = data["results"].toArray().first().toObject();

    object["completed"] = !object["completed"].toBool();
    reqId = client.update(object);

    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 2);
    QCOMPARE(spyError.count(), 0);
    response = spy[1][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    data = response->data();
    QCOMPARE(data["title"], object["title"]);
    QCOMPARE(data["objectType"], object["objectType"]);
    QCOMPARE(data["completed"], object["completed"]);
    QCOMPARE(response->backendStatus(), 200);
}

void tst_EnginioClient::update_invalidId()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject object;
    object["objectType"] = QString::fromUtf8("objects.todos");
    object["id"] = QString::fromUtf8("INVALID_ID");
    const EnginioReply* reqId = client.update(object);

    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 1);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->networkError(), QNetworkReply::ContentNotFoundError);
    QVERIFY(!response->errorString().isEmpty());
    QCOMPARE(reqId, spyError[0][0].value<EnginioReply*>());
    // TODO how ugly is that, improve JSON api
    QJsonObject data = response->data();
    QVERIFY(data["errors"].isArray());
    QVERIFY(!data["errors"].toArray()[0].toObject()["message"].toString().isEmpty());
    QVERIFY(!data["errors"].toArray()[0].toObject()["reason"].toString().isEmpty());
    QCOMPARE(response->backendStatus(), 404);
}

void tst_EnginioClient::remove_todos()
{
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject query;
    query["objectType"] = QString::fromUtf8("objects.todos");
    query["title"] = QString::fromUtf8("remove");
    query["completed"] = true;
    const EnginioReply* reqId = client.create(query);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    // confirm that a new object was created
    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);

    query["id"] = response->data()["id"];
    reqId = client.remove(query);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 2);
    QCOMPARE(spyError.count(), 0);
    response = spy[1][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);

    // it seems that object was deleted but lets try to query for it
    QJsonObject checkQuery;
    QJsonObject constructQuery;
    constructQuery["id"] = query["id"];
    checkQuery["query"] = constructQuery;
    checkQuery["objectType"] = query["objectType"];
    reqId = client.query(checkQuery);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 3);
    QCOMPARE(spyError.count(), 0);
    response = spy[2][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    QVERIFY(response->data()["results"].toArray().isEmpty());
}

void tst_EnginioClient::identity()
{
    {
        EnginioClient client;
        QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(sessionAuthenticated(EnginioReply*)));
        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

        identity.setUser("logintest");
        identity.setPassword("logintest");


        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
        client.setserviceUrl(EnginioTests::TESTAPP_URL);
        client.setIdentity(&identity);

        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);
        QCOMPARE(spyAuthError.count(), 0);
        QCOMPARE(client.authenticationState(), EnginioClient::Authenticated);

        QJsonObject token = spy[0][0].value<EnginioReply*>()->data();
        QVERIFY(token.contains("sessionToken"));
        QVERIFY(token.contains("user"));
        QVERIFY(token.contains("usergroups"));
    }
    {
        // Different initialization order
        EnginioClient client;
        QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(sessionAuthenticated(EnginioReply*)));
        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

        identity.setUser("logintest");
        identity.setPassword("logintest");

        client.setIdentity(&identity);
        client.setserviceUrl(EnginioTests::TESTAPP_URL);
        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);

        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);
        QCOMPARE(spyAuthError.count(), 0);
        QCOMPARE(client.authenticationState(), EnginioClient::Authenticated);
    }
    {
        // login / logout
        EnginioClient client;
        QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
        EnginioAuthentication identity;
        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

        identity.setUser("logintest");
        identity.setPassword("logintest");
        client.setIdentity(&identity);
        client.setserviceUrl(EnginioTests::TESTAPP_URL);
        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);

        QTRY_COMPARE(client.authenticationState(), EnginioClient::Authenticated);
        QCOMPARE(spyError.count(), 0);

        client.setIdentity(0);
        QTRY_COMPARE(client.authenticationState(), EnginioClient::NotAuthenticated);
        QCOMPARE(spyError.count(), 0);

        client.setIdentity(&identity);
        QTRY_COMPARE(client.authenticationState(), EnginioClient::Authenticated);
        QCOMPARE(spyError.count(), 0);
        QCOMPARE(spyAuthError.count(), 0);
    }
    {
        // change backend id
        EnginioClient client;
        QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
        EnginioAuthentication identity;
        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

        identity.setUser("logintest");
        identity.setPassword("logintest");
        client.setIdentity(&identity);
        client.setserviceUrl(EnginioTests::TESTAPP_URL);
        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);

        QTRY_COMPARE(client.authenticationState(), EnginioClient::Authenticated);
        QCOMPARE(spyError.count(), 0);

        client.setBackendId(QByteArray());
        client.setBackendId(EnginioTests::TESTAPP_ID);
        QTRY_COMPARE(client.authenticationState(), EnginioClient::Authenticated);
        QCOMPARE(spyError.count(), 0);
        QCOMPARE(spyAuthError.count(), 0);
    }
    {
        // fast identity change before initialization
        EnginioClient client;
        QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
        client.setserviceUrl(EnginioTests::TESTAPP_URL);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);

        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

        EnginioAuthentication identity1;
        EnginioAuthentication identity2;
        EnginioAuthentication identity3;

        identity1.setUser("logintest");
        identity1.setPassword("logintest");
        identity2.setUser("logintest2");
        identity2.setPassword("logintest2");
        identity3.setUser("logintest3");
        identity3.setPassword("logintest3");

        QCOMPARE(spyError.count(), 0);
        QCOMPARE(client.authenticationState(), EnginioClient::NotAuthenticated);

        for (uint i = 0; i < 4; ++i) {
            client.setIdentity(&identity1);
            client.setIdentity(&identity2);
            client.setIdentity(&identity3);
        }

        QCOMPARE(spyError.count(), 0);
        QCOMPARE(client.authenticationState(), EnginioClient::Authenticating);

        client.setBackendId(EnginioTests::TESTAPP_ID); // trigger authentication process

        QCOMPARE(client.authenticationState(), EnginioClient::Authenticating);
        QTRY_COMPARE(client.authenticationState(), EnginioClient::Authenticated);

        QCOMPARE(spyError.count(), 0);
        QCOMPARE(spyAuthError.count(), 0);
    }
    {
        // check if EnginoClient is properly detached from identity in destructor.
        EnginioClient *client = new EnginioClient;
        client->setserviceUrl(EnginioTests::TESTAPP_URL);
        client->setBackendSecret(EnginioTests::TESTAPP_SECRET);

        EnginioAuthentication identity;
        client->setIdentity(&identity);

        delete client;

        identity.setPassword("blah");
        identity.setUser("blah");
        // we should not crash
    }
    {
        // check if EnginoClient is properly detached from identity destructor.
        EnginioClient client;
        QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
        client.setserviceUrl(EnginioTests::TESTAPP_URL);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
        {
            EnginioAuthentication identity;
            client.setIdentity(&identity);
        }
        QVERIFY(!client.identity());
    }
}

void tst_EnginioClient::identity_invalid()
{
    {
        EnginioClient client;
        QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(sessionAuthenticated(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));
        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));

        identity.setUser("invalidLogin");
        identity.setPassword("invalidPassword");

        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
        client.setserviceUrl(EnginioTests::TESTAPP_URL);
        client.setIdentity(&identity);

        QTRY_COMPARE(spyAuthError.count(), 1);
        QTRY_COMPARE(spy.count(), 0);
        QCOMPARE(spyError.count(), 0);
        QCOMPARE(client.authenticationState(), EnginioClient::AuthenticationFailure);
    }
    {   // check if an old session is _not_ invalidated on an invalid re-loggin
        EnginioClient client;
        QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(sessionAuthenticated(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));
        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));

        identity.setUser("logintest");
        identity.setPassword("logintest");

        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
        client.setserviceUrl(EnginioTests::TESTAPP_URL);
        client.setIdentity(&identity);

        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);
        QTRY_COMPARE(spyAuthError.count(), 0);
        QCOMPARE(client.authenticationState(), EnginioClient::Authenticated);

        const QJsonObject identityReplyData = spy[0][0].value<EnginioReply*>()->data();

        // we are logged-in
        identity.setUser("invalidLogin");
        QTRY_COMPARE(spyAuthError.count(), 1);
        identity.setPassword("invalidPass");
        QTRY_COMPARE(spyAuthError.count(), 2);

        // get back to logged-in state
        identity.setUser("logintest2");
        QTRY_COMPARE(spyAuthError.count(), 3);
        QCOMPARE(spy.count(), 1);
        identity.setPassword("logintest2");
        QTRY_COMPARE(spy.count(), 2);
        QTRY_COMPARE(spyAuthError.count(), 3);

        QVERIFY(spy[1][0].value<EnginioReply*>()->data() != identityReplyData);
        QCOMPARE(client.authenticationState(), EnginioClient::Authenticated);
    }
}

void tst_EnginioClient::identity_afterLogout()
{
    qRegisterMetaType<QNetworkReply*>();
    EnginioClient client;

    EnginioAuthentication identity;
    identity.setUser("logintest");
    identity.setPassword("logintest");

    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);
    client.setIdentity(&identity);

    QVERIFY(client.networkManager());
    QTRY_COMPARE(client.authenticationState(), EnginioClient::Authenticated);

    // This may be fragile, we need real network reply to catch the header.
    QSignalSpy spy(client.networkManager(), SIGNAL(finished(QNetworkReply*)));
    QByteArray headerName = QByteArrayLiteral("Enginio-Backend-Session");

    // make a connection with a new session token
    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    const EnginioReply* reqId = client.query(obj);
    QVERIFY(reqId);
    QTRY_VERIFY(!reqId->data().isEmpty());
    CHECK_NO_ERROR(reqId);
    QCOMPARE(spy.count(), 1);

    QVERIFY(spy[0][0].value<QNetworkReply*>()->request().hasRawHeader(headerName));

    client.setIdentity(0);
    QTRY_COMPARE(client.authenticationState(), EnginioClient::NotAuthenticated);

    // unsecured data should be still accessible
    reqId = client.query(obj);
    QVERIFY(reqId);
    QTRY_VERIFY(!reqId->data().isEmpty());
    CHECK_NO_ERROR(reqId);
    QCOMPARE(spy.count(), 2);
    QVERIFY(!spy[1][0].value<QNetworkReply*>()->request().hasRawHeader(headerName));
}

void tst_EnginioClient::backendFakeReply()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spyClientFinished(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyClientError(&client, SIGNAL(error(EnginioReply*)));
    QSignalSpy spyReplyFinished(&client, SIGNAL(finished(EnginioReply*)));

    QJsonObject empty;
    QJsonObject objecTypeOnly;
    objecTypeOnly["objectType"] = QString::fromUtf8("objects.todos");

    client.query(empty, EnginioClient::ObjectOperation);
    client.query(empty, EnginioClient::ObjectAclOperation);
    client.query(objecTypeOnly, EnginioClient::ObjectAclOperation);
    client.query(empty, EnginioClient::UsergroupMembersOperation);

    client.update(empty, EnginioClient::ObjectOperation);
    client.update(empty, EnginioClient::ObjectAclOperation);
    client.update(objecTypeOnly, EnginioClient::ObjectAclOperation);
    client.update(empty, EnginioClient::UsergroupMembersOperation);

    client.remove(empty, EnginioClient::ObjectOperation);
    client.remove(empty, EnginioClient::ObjectAclOperation);
    client.remove(objecTypeOnly, EnginioClient::ObjectAclOperation);
    client.remove(empty, EnginioClient::UsergroupMembersOperation);

    client.search(empty);

    QTRY_COMPARE(spyClientFinished.count(), 13);
    QCOMPARE(spyClientError.count(), spyClientFinished.count());
    QCOMPARE(spyReplyFinished.count(), spyClientFinished.count());

    for (int i = 0; i < spyClientFinished.count(); ++i) {
        EnginioReply *reply = spyClientFinished[i][0].value<EnginioReply*>();
        QJsonObject data = reply->data();
        QVERIFY(!data.isEmpty());

        QVERIFY(!data["errors"].toArray()[0].toObject()["message"].toString().isEmpty());
        QVERIFY(!data["errors"].toArray()[0].toObject()["reason"].toString().isEmpty());

        QCOMPARE(reply->errorType(), EnginioReply::BackendError);
        QVERIFY(reply->networkError() != QNetworkReply::NoError);
        QCOMPARE(reply->backendStatus(), 400);

        QCOMPARE(spyReplyFinished[i][0].value<EnginioReply*>(), spyClientFinished[i][0].value<EnginioReply*>());
    }
}

void tst_EnginioClient::acl()
{
    // create an object
    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setserviceUrl(EnginioTests::TESTAPP_URL);

    EnginioAuthentication identity;
    identity.setUser("logintest");
    identity.setPassword("logintest");
    client.setIdentity(&identity);

    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QString id1, id2, id3; // ids of 3 different users id1 is our login
    QJsonParseError parseError;
    {
        QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
        QJsonObject userQuery = QJsonDocument::fromJson("{\"query\": {\"username\": {\"$in\": [\"logintest\", \"logintest2\",\"logintest3\"]}},"
                                                        "\"sort\": [{\"sortBy\":\"username\", \"direction\": \"asc\"}]}", &parseError).object();
        QCOMPARE(parseError.error, QJsonParseError::NoError);

        const EnginioReply* reqId = client.query(userQuery, EnginioClient::UserOperation);
        QVERIFY(reqId);
        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);
        QJsonArray data = reqId->data()["results"].toArray();
        QCOMPARE(data.count(), 3);
        id1 = data[0].toObject()["id"].toString();
        id2 = data[1].toObject()["id"].toString();
        id3 = data[2].toObject()["id"].toString();
        QVERIFY(!id1.isEmpty());
        QVERIFY(!id2.isEmpty());
        QVERIFY(!id3.isEmpty());
    }

    // wait for authentication, acl requires that
    QTRY_COMPARE(client.authenticationState(), EnginioClient::Authenticated);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));

    // create an object
    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    obj["title"] = QString::fromUtf8("test title");
    obj["completed"] = false;
    const EnginioReply* reqId = client.create(obj);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
    obj = response->data(); // so obj contains valid id

    // view acl of the object
    reqId = client.query(obj, EnginioClient::ObjectAclOperation);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 2);
    response = spy[1][0].value<EnginioReply*>();
    QCOMPARE(spyError.count(), 0);
    QCOMPARE(response, reqId);
    qDebug() << response->data();
    CHECK_NO_ERROR(response);

    QJsonObject data(response->data());
    QVERIFY(data["admin"].isArray());
    QVERIFY(data["read"].isArray());
    QVERIFY(data["delete"].isArray());
    QVERIFY(data["update"].isArray());

    // update acl of the object
    QString json = "{ \"read\": [ { \"id\": \"%3\", \"objectType\": \"users\" } ],"
                     "\"update\": [ { \"id\": \"%2\", \"objectType\": \"users\" } ],"
                     "\"admin\": [ { \"id\": \"%1\", \"objectType\": \"users\" } ] }";
    json = json.arg(id1, id2, id3);
    QJsonObject aclUpdate = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    QCOMPARE(parseError.error, QJsonParseError::NoError);
    aclUpdate["objectType"] = obj["objectType"];
    aclUpdate["id"] = obj["id"];

    reqId = client.update(aclUpdate, EnginioClient::ObjectAclOperation);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 3);
    QCOMPARE(spyError.count(), 0);
    response = spy[2][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);

    // confirm that the response is correct
    data = response->data();
    QVERIFY(data["admin"].isArray());
    QVERIFY(data["read"].isArray());
    QVERIFY(data["delete"].isArray());
    QVERIFY(data["update"].isArray());

    bool valid = false;
    foreach(QJsonValue value, data["read"].toArray()) {
        if (value.toObject()["id"] == id3) {
            valid = true;
            break;
        }
    }
    QVERIFY(valid);

    valid = false;
    foreach(QJsonValue value, data["update"].toArray()) {
        if (value.toObject()["id"] == id2) {
            valid = true;
            break;
        }
    }
    QVERIFY(valid);

    valid = false;
    foreach(QJsonValue value, data["admin"].toArray()) {
        if (value.toObject()["id"] == id1) {
            valid = true;
            break;
        }
    }
    QVERIFY(valid);

    // view acl again to confirm the change on the server side
    reqId = client.query(obj, EnginioClient::ObjectAclOperation);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 4);
    QCOMPARE(spyError.count(), 0);
    response = spy[3][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);

    data = response->data();
    QVERIFY(data["admin"].isArray());
    QVERIFY(data["read"].isArray());
    QVERIFY(data["delete"].isArray());
    QVERIFY(data["update"].isArray());
    valid = false;
    foreach(QJsonValue value, data["read"].toArray()) {
        if (value.toObject()["id"] == id3) {
            valid = true;
            break;
        }
    }
    QVERIFY(valid);

    valid = false;
    foreach(QJsonValue value, data["update"].toArray()) {
        if (value.toObject()["id"] == id2) {
            valid = true;
            break;
        }
    }
    QVERIFY(valid);

    valid = false;
    foreach(QJsonValue value, data["admin"].toArray()) {
        if (value.toObject()["id"] == id1) {
            valid = true;
            break;
        }
    }
    QVERIFY(valid);
    // it seems to work fine, let's delete the acl we created
    reqId = client.remove(aclUpdate, EnginioClient::ObjectAclOperation);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 5);
    QCOMPARE(spyError.count(), 0);
    response = spy[4][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    CHECK_NO_ERROR(response);
}

void tst_EnginioClient::sharingNetworkManager()
{
    // Check sharing of network acces manager in different enginio clients
    EnginioClient *e1 = new EnginioClient;
    EnginioClient *e2 = new EnginioClient;
    QCOMPARE(e1->networkManager(), e2->networkManager());

    // QNAM can not be shared accross threads
    struct EnginioInThread : public QThread {
        QNetworkAccessManager *_qnam;
        EnginioInThread(QNetworkAccessManager *qnam)
            : _qnam(qnam)
        {}
        void run()
        {
            EnginioClient e3;
            QVERIFY(_qnam != e3.networkManager());
        }
    } thread(e1->networkManager());
    thread.start();
    QVERIFY(thread.wait(10000));

    // check if deleting qnam creator is not invalidating qnam itself.
    delete e1;
    QVERIFY(e2->networkManager());
    e2->networkManager()->children(); // should not crash
    delete e2;
}

QTEST_MAIN(tst_EnginioClient)
#include "tst_enginioclient.moc"
