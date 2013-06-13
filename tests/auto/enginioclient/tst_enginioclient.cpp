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

class tst_EnginioClient: public QObject
{
    Q_OBJECT

public slots:
    void error(EnginioReply *reply) {
        qDebug() << "\n###\n" << reply << "\n###\n";
    }

private slots:
    void initTestCase();
    void init();
    void query_todos();
    void query_todos_filter();
    void query_todos_limit();
    void query_todos_count();
    void query_todos_sort();
    void users_crud();
    void query_users();
    void query_users_filter();
    void query_users_sort();
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
        client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
}

void tst_EnginioClient::init()
{
    if (EnginioTests::TESTAPP_ID.isEmpty() || EnginioTests::TESTAPP_SECRET.isEmpty() || EnginioTests::TESTAPP_URL.isEmpty())
        QFAIL("Needed environment variables ENGINIO_BACKEND_ID, ENGINIO_BACKEND_SECRET, ENGINIO_API_URL are not set!");
}

void tst_EnginioClient::query_todos()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QVERIFY(!response->data().isEmpty());
    QVERIFY(!response->data()["results"].isUndefined());
}

void tst_EnginioClient::query_todos_filter()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
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
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(data["results"].isArray());
    QCOMPARE(data["results"].toArray().count(), 1);
}

void tst_EnginioClient::query_todos_count()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(data.contains("count"));
}

void tst_EnginioClient::query_todos_sort()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    if (reqId->errorCode())
        qDebug() << reqId->data();
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
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
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    QJsonObject obj;
    const EnginioReply* reqId = client.query(obj, EnginioClient::UserOperation);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(!data["results"].isUndefined());
    QVERIFY(data["results"].toArray().count());
}

void tst_EnginioClient::query_users_sort()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
        QCOMPARE(response->errorCode(), QNetworkReply::NoError);
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
        if (reqId->errorCode())
            qDebug() << reqId->data();
        QCOMPARE(spyError.count(), 0);

        const EnginioReply *response = spy[0][0].value<EnginioReply*>();
        QCOMPARE(response, reqId);
        QCOMPARE(response->errorCode(), QNetworkReply::NoError);
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

void tst_EnginioClient::query_usersgroup_limit()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(!data["results"].isUndefined());
    QVERIFY(data["results"].toArray().count());
}

void tst_EnginioClient::query_usersgroup_count()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(data.contains("count"));
}

void tst_EnginioClient::query_usersgroup_sort()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
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
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(!data["results"].isUndefined());
    QVERIFY(data["results"].toArray().count() <= 1);
}

void tst_EnginioClient::query_usersgroupmembers_count()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(data.contains("count"));
}

void tst_EnginioClient::query_usersgroupmembers_sort()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
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
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QJsonObject data = response->data();
    QVERIFY(!data.isEmpty());
    QVERIFY(!data["results"].isUndefined());
    QVERIFY(!data["results"].toArray().count());
}

void tst_EnginioClient::search()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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

        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);

        const EnginioReply *response = spy[0][0].value<EnginioReply*>();

        QCOMPARE(response, reqId);
        QCOMPARE(response->errorCode(), QNetworkReply::NoError);
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
        QCOMPARE(response->errorCode(), QNetworkReply::NoError);
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
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
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
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
        QCOMPARE(response->errorCode(), QNetworkReply::NoError);
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
        QCOMPARE(reqId->errorCode(), QNetworkReply::NoError);
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
        QCOMPARE(reqId->errorCode(), QNetworkReply::NoError);
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
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    data = response->data();
    QCOMPARE(data["title"], object["title"]);
    QCOMPARE(data["objectType"], object["objectType"]);
    QCOMPARE(data["completed"], object["completed"]);
}

void tst_EnginioClient::update_invalidId()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::ContentNotFoundError);
    QVERIFY(!response->errorString().isEmpty());
    QCOMPARE(reqId, spyError[0][0].value<EnginioReply*>());
    // TODO how ugly is that, improve JSON api
    QJsonObject data = response->data();
    QVERIFY(!data["errors"].toArray()[0].toObject()["message"].toString().isEmpty());
    QVERIFY(!data["errors"].toArray()[0].toObject()["reason"].toString().isEmpty());
}

void tst_EnginioClient::remove_todos()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);

    query["id"] = response->data()["id"];
    reqId = client.remove(query);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 2);
    QCOMPARE(spyError.count(), 0);
    response = spy[1][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QVERIFY(response->data()["results"].toArray().isEmpty());
}

void tst_EnginioClient::identity()
{
    {
        EnginioClient client;
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(sessionAuthenticated()));
        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

        identity.setUser("logintest");
        identity.setPassword("logintest");


        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setIdentity(&identity);

        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);
        QCOMPARE(spyAuthError.count(), 0);
        QVERIFY(!client.identityToken().isEmpty());

        QJsonObject token = client.identityToken();
        QVERIFY(token.contains("sessionToken"));
        QVERIFY(token.contains("user"));
        QVERIFY(token.contains("usergroups"));
    }
    {
        // Different initialization order
        EnginioClient client;
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(sessionAuthenticated()));
        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

        identity.setUser("logintest");
        identity.setPassword("logintest");

        client.setIdentity(&identity);
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);

        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);
        QCOMPARE(spyAuthError.count(), 0);
        QVERIFY(!client.identityToken().isEmpty());
    }
    {
        // login / logout
        EnginioClient client;
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(identityTokenChanged(const QJsonObject &)));
        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

        identity.setUser("logintest");
        identity.setPassword("logintest");
        client.setIdentity(&identity);
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);

        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);
        QVERIFY(!client.identityToken().isEmpty());

        client.setIdentity(0);
        QTRY_COMPARE(spy.count(), 2);
        QCOMPARE(spyError.count(), 0);
        QVERIFY(client.identityToken().isEmpty());

        client.setIdentity(&identity);
        QTRY_COMPARE(spy.count(), 3);
        QCOMPARE(spyError.count(), 0);
        QVERIFY(!client.identityToken().isEmpty());
        QCOMPARE(spyAuthError.count(), 0);
    }
    {
        // change backend id
        EnginioClient client;
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(identityTokenChanged(const QJsonObject &)));
        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

        identity.setUser("logintest");
        identity.setPassword("logintest");
        client.setIdentity(&identity);
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);

        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);
        QVERIFY(!client.identityToken().isEmpty());

        client.setBackendId(QByteArray());
        client.setBackendId(EnginioTests::TESTAPP_ID);
        QTRY_COMPARE(spy.count(), 2);
        QCOMPARE(spyError.count(), 0);
        QCOMPARE(spyAuthError.count(), 0);
        QVERIFY(!client.identityToken().isEmpty());
    }
    {
        // fast identity change before initialization
        EnginioClient client;
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);

        QSignalSpy spy(&client, SIGNAL(identityTokenChanged(const QJsonObject &)));
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

        QCOMPARE(spy.count(), 0);
        QCOMPARE(spyError.count(), 0);
        QVERIFY(client.identityToken().isEmpty());

        for (uint i = 0; i < 4; ++i) {
            client.setIdentity(&identity1);
            client.setIdentity(&identity2);
            client.setIdentity(&identity3);
        }

        QCOMPARE(spy.count(), 0);
        QCOMPARE(spyError.count(), 0);
        QVERIFY(client.identityToken().isEmpty());

        client.setBackendId(EnginioTests::TESTAPP_ID); // trigger authentication process

        QTRY_COMPARE(spy.count(), 1);

        for (uint i = 0; spy.count() == 1 && i < 5; ++i)
            QTest::qWait(100);

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);
        QCOMPARE(spyAuthError.count(), 0);
        QVERIFY(!client.identityToken().isEmpty());
    }
    {
        // check if EnginoClient is properly detached from identity in destructor.
        EnginioClient *client = new EnginioClient;
        client->setApiUrl(EnginioTests::TESTAPP_URL);
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
        client.setApiUrl(EnginioTests::TESTAPP_URL);
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
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(sessionAuthenticated()));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));
        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));

        identity.setUser("invalidLogin");
        identity.setPassword("invalidPassword");

        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setIdentity(&identity);

        QTRY_COMPARE(spyAuthError.count(), 1);
        QTRY_COMPARE(spy.count(), 0);
        QCOMPARE(spyError.count(), 0);
        QVERIFY(client.identityToken().isEmpty());
    }
    {   // check if an old session is _not_ invalidated on an invalid re-loggin
        EnginioClient client;
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(sessionAuthenticated()));
        QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));
        QSignalSpy spyAuthError(&client, SIGNAL(sessionAuthenticationError(EnginioReply*)));

        identity.setUser("logintest");
        identity.setPassword("logintest");

        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setIdentity(&identity);

        QTRY_COMPARE(spy.count(), 1);
        QCOMPARE(spyError.count(), 0);
        QTRY_COMPARE(spyAuthError.count(), 0);
        QVERIFY(!client.identityToken().isEmpty());

        const QJsonObject identityToken = client.identityToken();

        // we are logged-in
        identity.setUser("invalidLogin");
        QTRY_COMPARE(spyAuthError.count(), 1);
        QCOMPARE(client.identityToken(), identityToken);
        identity.setPassword("invalidPass");
        QTRY_COMPARE(spyAuthError.count(), 2);
        QCOMPARE(client.identityToken(), identityToken);

        // get back to logged-in state
        identity.setUser("logintest2");
        QTRY_COMPARE(spyAuthError.count(), 3);
        QCOMPARE(spy.count(), 1);
        identity.setPassword("logintest2");
        QTRY_COMPARE(spy.count(), 2);
        QTRY_COMPARE(spyAuthError.count(), 3);

        QVERIFY(client.identityToken() != identityToken);
        QVERIFY(!client.identityToken().isEmpty());
    }
}

void tst_EnginioClient::acl()
{
    // create an object
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

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
    QTRY_VERIFY(!client.identityToken().isEmpty());

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    obj = response->data(); // so obj contains valid id

    // view acl of the object
    reqId = client.query(obj, EnginioClient::ObjectAclOperation);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 2);
    response = spy[1][0].value<EnginioReply*>();
    QCOMPARE(spyError.count(), 0);
    QCOMPARE(response, reqId);
    qDebug() << response->data();
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);

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
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
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
