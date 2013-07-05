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

#include <QtWidgets/qlistview.h>
#include <QtWidgets/qapplication.h>

#include <Enginio/enginioclient.h>
#include <Enginio/enginioreply.h>
#include <Enginio/enginiomodel.h>
#include <Enginio/enginioidentity.h>

#include "../common/common.h"

class tst_EnginioModel: public QObject
{
    Q_OBJECT

    QString _backendName;
    EnginioTests::EnginioBackendManager _backendManager;
    QByteArray _backendId;
    QByteArray _backendSecret;

public slots:
    void error(EnginioReply *reply) {
        qDebug() << "\n\n### ERROR";
        qDebug() << reply->errorString();
        reply->dumpDebugInfo();
        qDebug() << "\n###\n";
    }

private slots:
    void initTestCase();
    void cleanupTestCase();
    void ctor();
    void enginio_property();
    void query_property();
    void operation_property();
    void roleNames();
    void listView();
    void invalidRemove();
    void invalidSetProperty();
    void multpleConnections();
    void deletionReordered();
    void deleteTwiceTheSame();
    void updateAndDeleteReordered();
    void updateReordered();
};

void tst_EnginioModel::initTestCase()
{
    if (EnginioTests::TESTAPP_URL.isEmpty())
        QFAIL("Needed environment variable ENGINIO_API_URL is not set!");

    _backendName = QStringLiteral("EnginioClient") + QString::number(QDateTime::currentMSecsSinceEpoch());
    QVERIFY(_backendManager.createBackend(_backendName));

    QJsonObject apiKeys = _backendManager.backendApiKeys(_backendName, EnginioTests::TESTAPP_ENV);
    _backendId = apiKeys["backendId"].toString().toUtf8();
    _backendSecret = apiKeys["backendSecret"].toString().toUtf8();

    QVERIFY(!_backendId.isEmpty());
    QVERIFY(!_backendSecret.isEmpty());

    // The test operates on user data.
    EnginioTests::prepareTestUsersAndUserGroups(_backendId, _backendSecret);
}

void tst_EnginioModel::cleanupTestCase()
{
    QVERIFY(_backendManager.removeBackend(_backendName));
}

void tst_EnginioModel::ctor()
{
    {
        EnginioModel model1, model2(this);
        Q_UNUSED(model1);
        Q_UNUSED(model2);
    }

    QJsonObject query = QJsonDocument::fromJson("{\"foo\":\"invalid\"}").object();
    QVERIFY(!query.isEmpty());
    {   // check if destructor of a fully initilized EnginioClient detach fully initilized model
        EnginioModel model;
        model.setOperation(EnginioClient::ObjectOperation);
        model.setQuery(query);

        EnginioClient client;
        QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
        client.setBackendId(_backendId);
        client.setServiceUrl(EnginioTests::TESTAPP_URL);
        client.setBackendSecret(_backendSecret);
        model.setEnginio(&client);
    }
    {   // check if destructor of a fully initilized EnginioClient detach fully initilized model
        EnginioClient client;
        QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
        EnginioModel model;
        model.setOperation(EnginioClient::ObjectOperation);
        model.setQuery(query);
        client.setBackendId(_backendId);
        client.setServiceUrl(EnginioTests::TESTAPP_URL);
        client.setBackendSecret(_backendSecret);
        model.setEnginio(&client);
    }
}

void tst_EnginioModel::enginio_property()
{
    {
        EnginioClient client;
        QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
        EnginioModel model;
        // No initial value
        QCOMPARE(model.enginio(), static_cast<EnginioClient*>(0));

        QSignalSpy spy(&model, SIGNAL(enginioChanged(EnginioClient*)));
        model.setEnginio(&client);
        QCOMPARE(model.enginio(), &client);
        QTRY_COMPARE(spy.count(), 1);
    }
    {
        EnginioModel model;
        QSignalSpy spy(&model, SIGNAL(enginioChanged(EnginioClient*)));
        {
            EnginioClient client;
            QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
            model.setEnginio(&client);
            QCOMPARE(model.enginio(), &client);
            QTRY_COMPARE(spy.count(), 1);
            QCOMPARE(spy[0][0].value<EnginioClient*>(), &client);
        }
        QCOMPARE(model.enginio(), static_cast<EnginioClient*>(0));
        QTRY_COMPARE(spy.count(), 2);
        QCOMPARE(spy[1][0].value<EnginioClient*>(), static_cast<EnginioClient*>(0));
    }
}

void tst_EnginioModel::query_property()
{
    QJsonObject query = QJsonDocument::fromJson("{\"foo\":\"invalid\"}").object();
    QVERIFY(!query.isEmpty());
    EnginioModel model;
    QSignalSpy spy(&model, SIGNAL(queryChanged(QJsonObject)));

    // initial value is empty
    QVERIFY(model.query().isEmpty());

    model.setQuery(query);
    QCOMPARE(model.query(), query);
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy[0][0].value<QJsonObject>(), query);

    // try to set the same query again, it should not emit the signal
    model.setQuery(query);
    QCOMPARE(model.query(), query);
    QCOMPARE(spy.count(), 1);

    model.setQuery(QJsonObject());
    QTRY_COMPARE(spy.count(), 2);
    QVERIFY(model.query().isEmpty());
    QVERIFY(spy[1][0].value<QJsonObject>().isEmpty());
}

void tst_EnginioModel::operation_property()
{
    EnginioModel model;
    QSignalSpy spy(&model, SIGNAL(operationChanged(EnginioClient::Operation)));

    // check initial value
    QCOMPARE(model.operation(), EnginioClient::ObjectOperation);

    model.setOperation(EnginioClient::UserOperation);
    QCOMPARE(model.operation(), EnginioClient::UserOperation);
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy[0][0].value<EnginioClient::Operation>(), EnginioClient::UserOperation);

    // try to set the same operation again, it should not emit the signal
    model.setOperation(EnginioClient::UserOperation);
    QCOMPARE(model.operation(), EnginioClient::UserOperation);
    QCOMPARE(spy.count(), 1);

    // try to change it agian.
    model.setOperation(EnginioClient::UsergroupOperation);
    QTRY_COMPARE(spy.count(), 2);
    QCOMPARE(model.operation(), EnginioClient::UsergroupOperation);
    QCOMPARE(spy[1][0].value<EnginioClient::Operation>(), EnginioClient::UsergroupOperation);
}

void tst_EnginioModel::roleNames()
{
    struct EnginioModelChild: public EnginioModel
    {
        using EnginioModel::roleNames;
    } model;
    QVERIFY(model.roleNames().isEmpty()); // Initilial value

    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(_backendId);
    client.setBackendSecret(_backendSecret);
    client.setServiceUrl(EnginioTests::TESTAPP_URL);
    model.setEnginio(&client);

    QVERIFY(model.enginio() == &client);

    model.setOperation(EnginioClient::UserOperation);
    QJsonObject query = QJsonDocument::fromJson("{\"limit\":5}").object();
    model.setQuery(query);

    QTRY_COMPARE(model.rowCount(), 5);
    QHash<int, QByteArray> roles = model.roleNames();
    QSet<QByteArray> expectedRoles;
    expectedRoles << "updatedAt" << "objectType" << "id" << "username" << "createdAt" << "_synced";
    QSet<QByteArray> roleNames = roles.values().toSet();
    foreach(const QByteArray role, expectedRoles)
        QVERIFY(roleNames.contains(role));
}

void tst_EnginioModel::listView()
{
    QJsonObject query = QJsonDocument::fromJson("{\"limit\":2}").object();
    QVERIFY(!query.isEmpty());
    EnginioModel model;
    model.setQuery(query);
    model.setOperation(EnginioClient::UserOperation);

    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(_backendId);
    client.setBackendSecret(_backendSecret);
    client.setServiceUrl(EnginioTests::TESTAPP_URL);
    model.setEnginio(&client);

    QListView view;
    view.setModel(&model);
    view.show();

    QTRY_COMPARE(model.rowCount(), 2);
    QVERIFY(view.model() == &model);
    QVERIFY(view.indexAt(QPoint(1,1)).data().isValid());
}

struct ReplyCounter
{
    int &counter;
    ReplyCounter(int *storage)
        : counter(*storage)
    {}

    void operator ()(EnginioReply *)
    {
        ++counter;
    }
};

struct InvalidRemoveErrorChecker: public ReplyCounter
{
    InvalidRemoveErrorChecker(int *storage)
        : ReplyCounter(storage)
    {}

    void operator ()(EnginioReply *reply)
    {
        QVERIFY(reply->isError());
        QCOMPARE(reply->errorType(), EnginioReply::BackendError);
        QVERIFY(reply->networkError() != QNetworkReply::NoError);\
        QCOMPARE(reply->backendStatus(), 400);

        QJsonObject data = reply->data();
        QVERIFY(!data.isEmpty());

        QVERIFY(!data["errors"].toArray()[0].toObject()["message"].toString().isEmpty());
        QVERIFY(!data["errors"].toArray()[0].toObject()["reason"].toString().isEmpty());

        ReplyCounter::operator ()(reply);
    }
};
void tst_EnginioModel::invalidRemove()
{
    EnginioClient client;
    client.setBackendId(_backendId);
    client.setBackendSecret(_backendSecret);
    client.setServiceUrl(EnginioTests::TESTAPP_URL);

    EnginioModel model;
    model.setEnginio(&client);

    int counter = 0;
    InvalidRemoveErrorChecker replyCounter(&counter);
    EnginioReply *reply;

    reply = model.remove(0);
    QVERIFY(reply);
    QVERIFY(reply->parent());
    QObject::connect(reply, &EnginioReply::finished, replyCounter);

    reply = model.remove(-10);
    QVERIFY(reply);
    QVERIFY(reply->parent());
    QObject::connect(reply, &EnginioReply::finished, replyCounter);

    reply = model.remove(-1);
    QVERIFY(reply);
    QVERIFY(reply->parent());
    QObject::connect(reply, &EnginioReply::finished, replyCounter);

    reply = model.remove(model.rowCount());
    QVERIFY(reply);
    QVERIFY(reply->parent());
    QObject::connect(reply, &EnginioReply::finished, replyCounter);

    QTRY_COMPARE(counter, 4);
}

void tst_EnginioModel::invalidSetProperty()
{
    EnginioClient client;
    client.setBackendId(_backendId);
    client.setBackendSecret(_backendSecret);
    client.setServiceUrl(EnginioTests::TESTAPP_URL);

    EnginioModel model;
    model.setEnginio(&client);
    QJsonObject query = QJsonDocument::fromJson("{\"limit\":2}").object();
    model.setOperation(EnginioClient::UserOperation);
    model.setQuery(query);

    QTRY_VERIFY(model.rowCount());

    int counter = 0;
    ReplyCounter replyCounter(&counter);
    EnginioReply *reply;

    reply = model.setProperty(-10, "Blah", QVariant(123));
    QVERIFY(reply);
    QVERIFY(reply->parent());
    QObject::connect(reply, &EnginioReply::finished, replyCounter);

    reply = model.setProperty(-1, "Blah", QVariant(123));
    QVERIFY(reply);
    QVERIFY(reply->parent());
    QObject::connect(reply, &EnginioReply::finished, replyCounter);

    reply = model.setProperty(model.rowCount(), "Blah", QVariant(123));
    QVERIFY(reply);
    QVERIFY(reply->parent());
    QObject::connect(reply, &EnginioReply::finished, replyCounter);

    reply = model.setProperty(0, "Blah", QVariant(123)); // invalid Role
    QVERIFY(reply);
    QVERIFY(reply->parent());
    QObject::connect(reply, &EnginioReply::finished, replyCounter);

    QTRY_COMPARE(counter, 4);
}

struct EnginioClientConnectionSpy: public EnginioClient
{
    virtual void connectNotify(const QMetaMethod &signal) Q_DECL_OVERRIDE
    {
        counter[signal.name()] += 1;
    }
    virtual void disconnectNotify(const QMetaMethod &signal) Q_DECL_OVERRIDE
    {
        counter[signal.name()] -= 1;
    }

    QHash<QByteArray, int> counter;
};

struct EnginioModelConnectionSpy: public EnginioModel
{
    virtual void connectNotify(const QMetaMethod &signal) Q_DECL_OVERRIDE
    {
        counter[signal.name()] += 1;
    }
    virtual void disconnectNotify(const QMetaMethod &signal) Q_DECL_OVERRIDE
    {
        counter[signal.name()] -= 1;
    }

    QHash<QByteArray, int> counter;
};

void tst_EnginioModel::multpleConnections()
{
    {
        EnginioClientConnectionSpy client1;
        client1.setBackendId(_backendId);
        client1.setBackendSecret(_backendSecret);
        client1.setServiceUrl(EnginioTests::TESTAPP_URL);
        EnginioClientConnectionSpy client2;
        client2.setBackendId(_backendId);
        client2.setBackendSecret(_backendSecret);
        client2.setServiceUrl(EnginioTests::TESTAPP_URL);

        EnginioModelConnectionSpy model;
        for (int i = 0; i < 20; ++i) {
            model.setEnginio(&client1);
            model.setEnginio(&client2);
            model.setEnginio(0);
        }

        // The values here are not strict. Use qDebug() << model.counter; to see what
        // makes sense. Just to be sure 2097150 or 20 doesn't.
        QCOMPARE(model.counter["operationChanged"], 0);
        QCOMPARE(model.counter["queryChanged"], 0);
        QCOMPARE(model.counter["enginioChanged"], 0);

        // All of them are acctually disconnected but disconnectNotify is not called, it is
        // a known bug in Qt.
        QCOMPARE(client1.counter["finished"], 20);
        QCOMPARE(client1.counter["backendIdChanged"], 20);
        QCOMPARE(client1.counter["backendSecretChanged"], 20);
        QCOMPARE(client1.counter["destroyed"], 20);
        QCOMPARE(client2.counter["finished"], 20);
        QCOMPARE(client2.counter["backendIdChanged"], 20);
        QCOMPARE(client2.counter["backendSecretChanged"], 20);
        QCOMPARE(client2.counter["destroyed"], 20);
    }
    {
        EnginioClientConnectionSpy client;
        client.setBackendId(_backendId);
        client.setBackendSecret(_backendSecret);
        client.setServiceUrl(EnginioTests::TESTAPP_URL);

        EnginioModelConnectionSpy model;
        model.setEnginio(&client);
        QJsonObject query1, query2;
        query1.insert("objectType", QString("objects.todos"));
        query2.insert("objectType", QString("objects.blah"));
        for (int i = 0; i < 20; ++i) {
            model.setQuery(query1);
            model.setQuery(query2);
            model.setQuery(QJsonObject());
        }

        // The values here are not strict. Use qDebug() << model.counter; to see what
        // makes sense. Just to be sure 2097150 or 20 doesn't.
        QCOMPARE(model.counter["operationChanged"], 0);
        QCOMPARE(model.counter["queryChanged"], 0);
        QCOMPARE(model.counter["enginioChanged"], 0);
    }
    {
        EnginioClientConnectionSpy client;
        client.setBackendId(_backendId);
        client.setBackendSecret(_backendSecret);
        client.setServiceUrl(EnginioTests::TESTAPP_URL);

        EnginioModelConnectionSpy model;
        model.setEnginio(&client);
        QJsonObject query;
        query.insert("objectType", QString("objects.todos"));
        model.setQuery(query);

        for (int i = 0; i < 20; ++i) {
            model.setOperation(EnginioClient::ObjectOperation);
            model.setOperation(EnginioClient::ObjectAclOperation);
        }

        // The values here are not strict. Use qDebug() << model.counter; to see what
        // makes sense. Just to be sure 2097150 or 20 doesn't.
        QCOMPARE(model.counter["operationChanged"], 0);
        QCOMPARE(model.counter["queryChanged"], 0);
        QCOMPARE(model.counter["enginioChanged"], 0);
    }
}

struct ReorderWaitForReply : public ReplyCounter
{
    bool &_trigger;
    ReorderWaitForReply(int *storage, bool *trigger, EnginioReply *ptr = 0)
        : ReplyCounter(storage)
        , _trigger(*trigger)
    {
        waitPointer = ptr;
    }

    void operator ()(EnginioReply *reply)
    {
        if (reply == waitPointer)
            _trigger = false;
        ReplyCounter::operator ()(reply);
    }
    static EnginioReply *waitPointer;
};
EnginioReply *ReorderWaitForReply::waitPointer = 0;

static bool reorderDelay = true;
bool reorderDelayer(EnginioReply */*reply*/)
{
    return reorderDelay;
}

void tst_EnginioModel::deletionReordered()
{
    QJsonObject query = QJsonDocument::fromJson("{\"limit\":2}").object();
    QVERIFY(!query.isEmpty());
    EnginioModel model;
    model.setQuery(query);
    model.setOperation(EnginioClient::UserOperation);

    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(_backendId);
    client.setBackendSecret(_backendSecret);
    client.setServiceUrl(EnginioTests::TESTAPP_URL);
    model.setEnginio(&client);

    QTRY_COMPARE(model.rowCount(), int(query["limit"].toDouble()));
    QVERIFY(model.rowCount() >= 2);

    EnginioReply *r2 = model.remove(model.rowCount() - 1);
    EnginioReply *r1 = model.remove(0);

    QVERIFY(!r1->isError());
    QVERIFY(!r2->isError());

    r2->setDelayFinishedSignalFunction(reorderDelayer);

    int counter = 0;
    reorderDelay = true;
    ReorderWaitForReply replyCounter(&counter, &reorderDelay, r1);
    QObject::connect(r1, &EnginioReply::finished, replyCounter);
    QObject::connect(r2, &EnginioReply::finished, replyCounter);

    QTRY_COMPARE(counter, 2);
}

struct MessageHandler
{
    MessageHandler(QString messageBegining)
        : oldMsgHandler(qInstallMessageHandler(handler))
    {
        msgBegining = messageBegining;
        ok = false;
    }

    ~MessageHandler()
    {
        qInstallMessageHandler(oldMsgHandler);
    }

    QtMessageHandler oldMsgHandler;

    static void handler(QtMsgType type, const QMessageLogContext & /*ctxt*/, const QString &msg)
    {
        Q_UNUSED(type);
        ok = msg.startsWith(msgBegining);
        QVERIFY2(ok, (QString::fromLatin1("Message is not started correctly: '") + msg + '\'').toLatin1().constData());
    }
    static QString msgBegining;
    static bool ok;
};
bool MessageHandler::ok;
QString MessageHandler::msgBegining;

void tst_EnginioModel::deleteTwiceTheSame()
{
    QJsonObject query = QJsonDocument::fromJson("{\"limit\":1}").object();
    QVERIFY(!query.isEmpty());
    EnginioModel model;
    model.setQuery(query);
    model.setOperation(EnginioClient::UserOperation);

    EnginioClient client;
    client.setBackendId(_backendId);
    client.setBackendSecret(_backendSecret);
    client.setServiceUrl(EnginioTests::TESTAPP_URL);
    model.setEnginio(&client);

    QTRY_COMPARE(model.rowCount(), int(query["limit"].toDouble()));

    EnginioReply *r2 = model.remove(model.rowCount() - 1);
    EnginioReply *r1 = model.remove(model.rowCount() - 1);

    QVERIFY(!r1->isError());
    QVERIFY(!r2->isError());

    int counter = 0;
    ReplyCounter replyCounter(&counter);
    QObject::connect(r1, &EnginioReply::finished, replyCounter);
    QObject::connect(r2, &EnginioReply::finished, replyCounter);

    MessageHandler handler("The same row was removed twice, removed object was: ");
    QTRY_COMPARE(counter, 2);

    // That is flaky sometimes server send us two positive message about deletion and
    // sometimes one positive and one 404. In the first case we expect model to handle
    // that gently with a warning.
    QVERIFY(MessageHandler::ok || r1->backendStatus() == 404 || r2->backendStatus() == 404);
}


void tst_EnginioModel::updateAndDeleteReordered()
{
    QJsonObject query = QJsonDocument::fromJson("{\"limit\":1}").object();
    QVERIFY(!query.isEmpty());
    EnginioModel model;
    model.setQuery(query);
    model.setOperation(EnginioClient::UserOperation);

    EnginioClient client;
    client.setBackendId(_backendId);
    client.setBackendSecret(_backendSecret);
    client.setServiceUrl(EnginioTests::TESTAPP_URL);
    model.setEnginio(&client);

    QTRY_COMPARE(model.rowCount(), int(query["limit"].toDouble()));

    EnginioReply *r2 = model.setProperty(model.rowCount() - 1, "email", "email@email.com");
    EnginioReply *r1 = model.remove(model.rowCount() - 1);

    QVERIFY(!r1->isError());
    QVERIFY(!r2->isError());

    int counter = 0;
    ReplyCounter replyCounter(&counter);
    QObject::connect(r1, &EnginioReply::finished, replyCounter);
    QObject::connect(r2, &EnginioReply::finished, replyCounter);

    MessageHandler handler("Trying to update a removed object:");
    QTRY_COMPARE(counter, 2);

    // That is flaky sometimes server send us two positive message and
    // sometimes one positive and one 404. In the first case we expect model to handle
    // that gently with a warning.
    QVERIFY(MessageHandler::ok || r1->backendStatus() == 404 || r2->backendStatus() == 404);
}

void tst_EnginioModel::updateReordered()
{
    QJsonObject query = QJsonDocument::fromJson("{\"limit\":1}").object();
    QVERIFY(!query.isEmpty());
    EnginioModel model;
    model.setQuery(query);
    model.setOperation(EnginioClient::UserOperation);

    EnginioClient client;
    client.setBackendId(_backendId);
    client.setBackendSecret(_backendSecret);
    client.setServiceUrl(EnginioTests::TESTAPP_URL);
    model.setEnginio(&client);

    QTRY_COMPARE(model.rowCount(), int(query["limit"].toDouble()));
    qDebug() << "orginal" << model.data(model.index(0)).value<QJsonValue>().toObject();

    int counter = 0;
    reorderDelay = true;

    EnginioReply *r2 = model.setProperty(0, "email", "email2@email.com");
    QVERIFY(!r2->isError());
    r2->setDelayFinishedSignalFunction(reorderDelayer);
    ReorderWaitForReply replyCounter(&counter, &reorderDelay);
    QObject::connect(r2, &EnginioReply::finished, replyCounter);

    QTRY_VERIFY(!r2->data().isEmpty()); // at this point r2 is done but finished signal is not emited
    QTRY_COMPARE(counter, 0);

    EnginioReply *r1 = model.setProperty(0, "email", "email1@email.com");
    QVERIFY(!r1->isError());

    ReorderWaitForReply::waitPointer = r1; // r1 finish signal will trigger r2 finish signal
    QObject::connect(r1, &EnginioReply::finished, replyCounter);

    QTRY_COMPARE(counter, 2);

    QDateTime r1UpdatedAt = QDateTime::fromString(r1->data()["updatedAt"].toString(), Qt::ISODate);
    QDateTime r2UpdatedAt = QDateTime::fromString(r2->data()["updatedAt"].toString(), Qt::ISODate);

    QVERIFY(r2UpdatedAt < r1UpdatedAt);
    QCOMPARE(model.data(model.index(0)).value<QJsonValue>().toObject(), r1->data());
}


QTEST_MAIN(tst_EnginioModel)
#include "tst_enginiomodel.moc"
