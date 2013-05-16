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

#include <Enginio/enginioclient.h>
#include <Enginio/enginioreply.h>
#include <Enginio/enginioidentity.h>

#include "../common/common.h"

class tst_EnginioClient: public QObject
{
    Q_OBJECT

private slots:
    void query_todos();
    void query_todos_filter();
    void query_todos_limit();
    void query_users();
    void query_users_filter();
    void search();
    void create_todos();
    void user_crud();
    void update_todos();
    void update_invalidId();
    void remove_todos();
    void identity();
    void acl();
    void file();
};

void tst_EnginioClient::query_todos()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply *)));

    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    const EnginioReply* reqId = client.query(obj);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);

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

    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    obj["query"] = QJsonDocument::fromJson("{\"completed\": true}").object();
    const EnginioReply* reqId = client.query(obj);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QVERIFY(!response->data().isEmpty());
    QVERIFY(!response->data()["results"].isUndefined());
    QJsonArray array = response->data()["results"].toArray();
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

    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    obj["limit"] = 1;
    const EnginioReply* reqId = client.query(obj);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QVERIFY(!response->data().isEmpty());
    QVERIFY(response->data()["results"].isArray());
    QCOMPARE(response->data()["results"].toArray().count(), 1);
}

void tst_EnginioClient::query_users()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));

    QJsonObject obj;
    const EnginioReply* reqId = client.query(obj, EnginioClient::UserOperation);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QVERIFY(!response->data().isEmpty());
    QVERIFY(!response->data()["results"].isUndefined());
    QVERIFY(response->data()["results"].toArray().count());
}

void tst_EnginioClient::query_users_filter()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));

    QJsonObject obj = QJsonDocument::fromJson("{\"query\":{\"username\": \"john\"}}").object();
    QVERIFY(!obj.isEmpty());
    const EnginioReply* reqId = client.query(obj, EnginioClient::UserOperation);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QVERIFY(!response->data().isEmpty());
    QVERIFY(!response->data()["results"].isUndefined());
    QVERIFY(!response->data()["results"].toArray().count());
}

void tst_EnginioClient::search()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);
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

        const EnginioReply *response = spy[0][0].value<EnginioReply*>();

        QCOMPARE(response, reqId);
        QCOMPARE(response->errorCode(), QNetworkReply::NoError);
        QVERIFY(!response->data().isEmpty());
        QVERIFY(!response->data()["results"].isUndefined());
        resultCount1 = response->data()["results"].toArray().count();
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

        const EnginioReply *response = spy[0][0].value<EnginioReply*>();

        QCOMPARE(response, reqId);
        QCOMPARE(response->errorCode(), QNetworkReply::NoError);
        QVERIFY(!response->data().isEmpty());
        QVERIFY(!response->data()["results"].isUndefined());
        int resultCount2 = response->data()["results"].toArray().count();
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

    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    obj["title"] = QString::fromUtf8("test title");
    obj["completed"] = false;
    const EnginioReply* reqId = client.create(obj);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QVERIFY(!response->data().isEmpty());
    QCOMPARE(response->data()["completed"], obj["completed"]);
    QCOMPARE(response->data()["title"], obj["title"]);
    QCOMPARE(response->data()["objectType"], obj["objectType"]);
}

void tst_EnginioClient::user_crud()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));

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
    }
}

void tst_EnginioClient::update_todos()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));

    QJsonObject query;
    query["objectType"] = QString::fromUtf8("objects.todos");
    query["limit"] = 1;
    const EnginioReply* reqId = client.query(query);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QVERIFY(!response->data().isEmpty());
    QJsonObject object = response->data()["results"].toArray().first().toObject();

    object["completed"] = !object["completed"].toBool();
    reqId = client.update(object);

    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 2);
    response = spy[1][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    QCOMPARE(response->data()["title"], object["title"]);
    QCOMPARE(response->data()["objectType"], object["objectType"]);

    QCOMPARE(response->data()["completed"], object["completed"]);
}

void tst_EnginioClient::update_invalidId()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));

    QJsonObject object;
    object["objectType"] = QString::fromUtf8("objects.todos");
    object["id"] = QString::fromUtf8("INVALID_ID");
    const EnginioReply* reqId = client.update(object);

    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 1);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::ContentNotFoundError);
    QVERIFY(!response->errorString().isEmpty());
    // TODO how ugly is that, improve JSON api
    QVERIFY(!response->data()["errors"].toArray()[0].toObject()["message"].toString().isEmpty());
    QVERIFY(!response->data()["errors"].toArray()[0].toObject()["reason"].toString().isEmpty());
}

void tst_EnginioClient::remove_todos()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));

    QJsonObject query;
    query["objectType"] = QString::fromUtf8("objects.todos");
    query["title"] = QString::fromUtf8("remove");
    query["completed"] = true;
    const EnginioReply* reqId = client.create(query);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 1);

    // confirm that a new object was created
    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);

    query["id"] = response->data()["id"];
    reqId = client.remove(query);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 2);
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

        identity.setUser("logintest");
        identity.setPassword("logintest");


        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setIdentity(&identity);

        QTRY_COMPARE(spy.count(), 1);
        QVERIFY(!client.sessionToken().isEmpty());
    }
    {
        // Different initialization order
        EnginioClient client;
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(sessionAuthenticated()));

        identity.setUser("logintest");
        identity.setPassword("logintest");


        client.setIdentity(&identity);
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);

        QTRY_COMPARE(spy.count(), 1);
        QVERIFY(!client.sessionToken().isEmpty());
    }
    {
        // login / logout
        EnginioClient client;
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(sessionTokenChanged()));

        identity.setUser("logintest");
        identity.setPassword("logintest");
        client.setIdentity(&identity);
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);

        QTRY_COMPARE(spy.count(), 1);
        QVERIFY(!client.sessionToken().isEmpty());

        client.setIdentity(0);
        QTRY_COMPARE(spy.count(), 2);
        QVERIFY(client.sessionToken().isEmpty());

        client.setIdentity(&identity);
        QTRY_COMPARE(spy.count(), 3);
        QVERIFY(!client.sessionToken().isEmpty());
    }
    {
        // change backend id
        EnginioClient client;
        EnginioAuthentication identity;
        QSignalSpy spy(&client, SIGNAL(sessionTokenChanged()));

        identity.setUser("logintest");
        identity.setPassword("logintest");
        client.setIdentity(&identity);
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);

        QTRY_COMPARE(spy.count(), 1);
        QVERIFY(!client.sessionToken().isEmpty());

        client.setBackendId(QString());
        client.setBackendId(EnginioTests::TESTAPP_ID);
        QTRY_COMPARE(spy.count(), 2); // we got another EnginioClient::clientInitialized signal TODO is it ok?
        QVERIFY(!client.sessionToken().isEmpty());
    }
    {
        // fast identity change before initialization
        EnginioClient client;
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);

        QSignalSpy spy(&client, SIGNAL(sessionTokenChanged()));
        QSignalSpy spyInit(&client, SIGNAL(clientInitialized()));

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
        QVERIFY(client.sessionToken().isEmpty());

        for (uint i = 0; i < 4; ++i) {
            client.setIdentity(&identity1);
            client.setIdentity(&identity2);
            client.setIdentity(&identity3);
        }

        QCOMPARE(spyInit.count(), 0);
        QCOMPARE(spy.count(), 0);
        QVERIFY(client.sessionToken().isEmpty());

        client.setBackendId(EnginioTests::TESTAPP_ID); // trigger clientInitialized signal

        QTRY_COMPARE(spyInit.count(), 1);
        QTRY_COMPARE(spy.count(), 1);

        for (uint i = 0; spy.count() == 1 && i < 5; ++i)
            QTest::qWait(100);

        QCOMPARE(spy.count(), 1);
        QVERIFY(!client.sessionToken().isEmpty());
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

    QString id1, id2, id3; // ids of 3 different users id1 is our login
    QJsonParseError parseError;
    {
        QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));
        QJsonObject userQuery = QJsonDocument::fromJson("{\"query\": {\"username\": {\"$in\": [\"logintest\", \"logintest2\",\"logintest3\"]}},"
                                 "\"sort\": {\"sortBy\":\"username\", \"direction\": \"asc\"}}", &parseError).object();
        QCOMPARE(parseError.error, QJsonParseError::NoError);

        const EnginioReply* reqId = client.query(userQuery, EnginioClient::UserOperation);
        QVERIFY(reqId);
        QTRY_COMPARE(spy.count(), 1);
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
    QTRY_VERIFY(!client.sessionToken().isEmpty());

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply*)));

    // create an object
    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    obj["title"] = QString::fromUtf8("test title");
    obj["completed"] = false;
    const EnginioReply* reqId = client.create(obj);
    QVERIFY(reqId);

    QTRY_COMPARE(spy.count(), 1);

    const EnginioReply *response = spy[0][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
    obj = response->data(); // so obj contains valid id

    // view acl of the object
    reqId = client.query(obj, EnginioClient::ObjectAclOperation);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 2);
    response = spy[1][0].value<EnginioReply*>();
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

    // TODO FIXME!
    QSKIP("DELETE requires data, but QNAM currently do not support it");
    // it seems to work fine, let's delete the acl we created
    reqId = client.remove(obj, EnginioClient::ObjectAclOperation);
    QVERIFY(reqId);
    QTRY_COMPARE(spy.count(), 5);
    response = spy[4][0].value<EnginioReply*>();
    QCOMPARE(response, reqId);
    QCOMPARE(response->errorCode(), QNetworkReply::NoError);
}

void tst_EnginioClient::file()
{
    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply *)));

    // Create a new object
    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.todos");
    obj["title"] = QString::fromUtf8("Object With File");
    const EnginioReply* req = client.create(obj);
    QVERIFY(req);

    QTRY_COMPARE(spy.count(), 1);

    const EnginioReply *responseObjectCreation = spy[0][0].value<EnginioReply*>();
    QCOMPARE(responseObjectCreation, req);
    QCOMPARE(responseObjectCreation->errorCode(), QNetworkReply::NoError);
    QVERIFY(!responseObjectCreation->data().isEmpty());
    QCOMPARE(responseObjectCreation->data()["title"], obj["title"]);
    QCOMPARE(responseObjectCreation->data()["objectType"], obj["objectType"]);
    QString id = responseObjectCreation->data()["id"].toString();
    QVERIFY(!id.isEmpty());

    QJsonObject object;
    object["id"] = id;
    object["objectType"] = QStringLiteral("objects.todos");

    // Attach file to the object

    // FIXME: make this work for out of source builds
    // FIXME: consider this url mess
    QString path = "file://" + QStringLiteral(TEST_FILE_PATH);
    QVERIFY(QFile::exists(QStringLiteral(TEST_FILE_PATH)));
    const EnginioReply* req2 = client.uploadFile(object, QUrl(path));
    QVERIFY(req2);

    QTRY_COMPARE(spy.count(), 2);
    const EnginioReply *responseUpload = spy[1][0].value<EnginioReply*>();
    qDebug() << responseUpload;
    // FIXME: write test here

    // Query including files
    QJsonObject obj2;
    obj2 = QJsonDocument::fromJson(
                "{\"include\": {\"file\": {}},"
                 "\"objectType\": \"objects.todos\","
                 "\"query\": {\"id\": \"" + id.toUtf8() + "\"}}").object();

    const EnginioReply *reply = client.query(obj2);
    QVERIFY(reply);

    QTRY_COMPARE(spy.count(), 3);
    const EnginioReply *responseQuery = spy[2][0].value<EnginioReply*>();
    QVERIFY(responseQuery->data()["results"].isArray());
    qDebug() << endl << responseQuery->data()["results"].toArray().first().toObject();

    QVERIFY(responseQuery->data()["results"].toArray().first().toObject()["file"].isObject());
    QVERIFY(!responseQuery->data()["results"].toArray().first().toObject()["file"].toObject()["url"].toString().isEmpty());
    qDebug() << "Download from here: " << client.apiUrl() << responseQuery->data()["results"].toArray().first().toObject()["file"].toObject()["url"].toString();

    // Download
    const EnginioReply* reqDownload = client.downloadFile(object);
    QVERIFY(reqDownload);

    QTRY_COMPARE(spy.count(), 4);
    const EnginioReply *responseDownload = spy[3][0].value<EnginioReply*>();
    qDebug() << responseDownload;
    qDebug() << "Download: " << responseDownload;

}

QTEST_MAIN(tst_EnginioClient)
#include "tst_enginioclient.moc"
