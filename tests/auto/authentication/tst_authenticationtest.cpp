#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <QDebug>

#include "../common/common.h"
#include "enginioclient.h"
#include "enginioerror.h"
#include "enginioidentityauthoperation.h"
#include "enginiojsonobject.h"
#include "enginioobjectoperation.h"

class AuthenticationTest : public QObject
{
    Q_OBJECT

public:
    AuthenticationTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testNonExistingUsername();
    void testWrongBackendId_data();
    void testWrongBackendId();
    void testAuthenticationSequence();
    void testAttachToSession();
    void testCustomUserProperties();

private:
    QPointer<EnginioClient> m_client;
};

AuthenticationTest::AuthenticationTest()
{
    qsrand((uint)QTime::currentTime().msec());
    qRegisterMetaType<EnginioError*>(); // required by QSignalSpy
}

void AuthenticationTest::initTestCase()
{
    m_client = new EnginioClient(EnginioTests::TESTAPP_ID,
                                 EnginioTests::TESTAPP_SECRET,
                                 this);
    QVERIFY2(m_client, "Client creation failed");
    m_client->setApiUrl(EnginioTests::TESTAPP_URL);
}

void AuthenticationTest::cleanupTestCase()
{
}

void AuthenticationTest::testNonExistingUsername()
{
    QVERIFY2(m_client, "Null client");
    QSignalSpy sessionAuthSpy(m_client, SIGNAL(sessionAuthenticated()));
    QSignalSpy sessionTermSpy(m_client, SIGNAL(sessionTerminated()));

    EnginioIdentityAuthOperation *op = new EnginioIdentityAuthOperation(m_client);
    QVERIFY2(op, "Operation creation failed");
    op->loginWithUsernameAndPassword("nonexistingusername", "pwd");

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 1);
    QVERIFY(op->error());
    QCOMPARE(op->error()->httpCode(), 401);
    QCOMPARE(sessionAuthSpy.count(), 0);
    QCOMPARE(sessionTermSpy.count(), 0);

    delete op;
}

void AuthenticationTest::testWrongBackendId_data()
{
    QTest::addColumn<QString>("backendId");
    QTest::addColumn<QString>("backendSecret");

    QTest::newRow("wrong ID") << "123123123" << EnginioTests::TESTAPP_SECRET;
    QTest::newRow("wrong secret") << EnginioTests::TESTAPP_ID << "123123123";
    QTest::newRow("all wrong") << "123123123" << "123123123";
}

void AuthenticationTest::testWrongBackendId()
{
    QFETCH(QString, backendId);
    QFETCH(QString, backendSecret);

    EnginioClient *client = new EnginioClient(backendId, backendSecret);
    QVERIFY2(client, "Client creation failed");
    client->setApiUrl(EnginioTests::TESTAPP_URL);

    QSignalSpy sessionAuthSpy(client, SIGNAL(sessionAuthenticated()));
    QSignalSpy sessionTermSpy(client, SIGNAL(sessionTerminated()));

    /* Try to login with wrong credentials */

    EnginioIdentityAuthOperation *authOp = new EnginioIdentityAuthOperation(client);
    QVERIFY2(authOp, "Operation creation failed");
    authOp->loginWithUsernameAndPassword("nonexistingusername", "pwd");

    QSignalSpy finishedSpy1(authOp, SIGNAL(finished()));
    QSignalSpy errorSpy1(authOp, SIGNAL(error(EnginioError*)));

    authOp->execute();

    QVERIFY2(finishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 1);
    QVERIFY(authOp->error());
    QCOMPARE(authOp->error()->httpCode(), 400);

    QCOMPARE(sessionAuthSpy.count(), 0);
    QCOMPARE(sessionTermSpy.count(), 0);

    delete authOp;

    /* Try to create new object */

    EnginioJsonObject *object = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    QVERIFY2(object, "Object creation failed");
    object->insert("stringValue", EnginioTests::randomString(10));
    object->insert("intValue", 34);
    QVERIFY2(object->id().isEmpty(), "New object's ID not empty");

    EnginioObjectOperation *objOp = new EnginioObjectOperation(client);
    objOp->create(object);

    QSignalSpy finishedSpy2(objOp, SIGNAL(finished()));
    QSignalSpy errorSpy2(objOp, SIGNAL(error(EnginioError*)));

    objOp->execute();

    QVERIFY2(finishedSpy2.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(finishedSpy2.count(), 1);
    QCOMPARE(errorSpy2.count(), 1);
    QVERIFY(objOp->error());
    QCOMPARE(objOp->error()->httpCode(), 400);

    delete object;
    delete objOp;
    delete client;
}

void AuthenticationTest::testAuthenticationSequence()
{
    QVERIFY2(m_client, "Null client");
    QSignalSpy sessionAuthSpy(m_client, SIGNAL(sessionAuthenticated()));
    QSignalSpy sessionTermSpy(m_client, SIGNAL(sessionTerminated()));

    /* Create user object */

    EnginioJsonObject *user = new EnginioJsonObject("users");
    user->insert("username", EnginioTests::randomString(15));
    user->insert("firstName", QStringLiteral("firstName_testAuthSequence"));
    user->insert("lastName", QStringLiteral("lastName_testAuthSequence"));
    user->insert("password", QStringLiteral("password"));
    QVERIFY(EnginioTests::createObject(m_client, user));
    QVERIFY(!user->id().isEmpty());

    /* Login with faulty password */

    EnginioIdentityAuthOperation *op = new EnginioIdentityAuthOperation(m_client);
    QVERIFY2(op, "Operation creation failed");
    op->loginWithUsernameAndPassword(user->value("username").toString(),
                                     "invalidpassword");

    QSignalSpy finishedSpy1(op, SIGNAL(finished()));
    QSignalSpy errorSpy1(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 1);
    QVERIFY(op->error());
    QCOMPARE(op->error()->httpCode(), 401);
    QCOMPARE(sessionAuthSpy.count(), 0);
    QCOMPARE(sessionTermSpy.count(), 0);

    delete op;

    /* Login with good credentials */

    op = new EnginioIdentityAuthOperation(m_client);
    QVERIFY2(op, "Operation creation failed");
    op->loginWithUsernameAndPassword(user->value("username").toString(),
                                     user->value("password").toString());

    QSignalSpy finishedSpy2(op, SIGNAL(finished()));
    QSignalSpy errorSpy2(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy2.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(sessionAuthSpy.count(), 1);
    QCOMPARE(sessionTermSpy.count(), 0);
    QCOMPARE(finishedSpy2.count(), 1);
    QCOMPARE(errorSpy2.count(), 0);
    QVERIFY2(!m_client->sessionToken().isEmpty(), "Empty session token");

    const EnginioJsonObject *loggedInUser = dynamic_cast<const EnginioJsonObject*>(
                op->loggedInUser());
    QVERIFY(loggedInUser);
    QCOMPARE(loggedInUser->value("username").toString(),
             user->value("username").toString());
    QCOMPARE(loggedInUser->value("firstName").toString(),
             user->value("firstName").toString());
    QCOMPARE(loggedInUser->value("lastName").toString(),
             user->value("lastName").toString());
    QVERIFY(loggedInUser->value("password").isUndefined());

    delete op;

    /* Logout */

    op = new EnginioIdentityAuthOperation(m_client);
    QVERIFY2(op, "Operation creation failed");
    op->logout();

    QSignalSpy finishedSpy3(op, SIGNAL(finished()));
    QSignalSpy errorSpy3(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy3.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(sessionAuthSpy.count(), 1);
    QCOMPARE(sessionTermSpy.count(), 1);
    QCOMPARE(finishedSpy3.count(), 1);
    QCOMPARE(errorSpy3.count(), 0);
    QVERIFY2(m_client->sessionToken().isEmpty(), "Non-empty session token");

    delete op;

    /* Logout again */

    op = new EnginioIdentityAuthOperation(m_client);
    QVERIFY2(op, "Operation creation failed");
    op->logout();

    QSignalSpy finishedSpy4(op, SIGNAL(finished()));
    QSignalSpy errorSpy4(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy4.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(sessionAuthSpy.count(), 1);
    QCOMPARE(sessionTermSpy.count(), 1);
    QCOMPARE(finishedSpy4.count(), 1);
    QCOMPARE(errorSpy4.count(), 1);
    QVERIFY(op->error());
    QCOMPARE(op->error()->httpCode(), 401);

    delete op;
    delete user;
}

void AuthenticationTest::testAttachToSession()
{
    QVERIFY2(m_client, "Null client");
    QSignalSpy sessionAuthSpy1(m_client, SIGNAL(sessionAuthenticated()));
    QSignalSpy sessionTermSpy1(m_client, SIGNAL(sessionTerminated()));

    /* Create user object */

    EnginioJsonObject *user = new EnginioJsonObject("users");
    user->insert("username", EnginioTests::randomString(15));
    user->insert("firstName", QStringLiteral("firstName_AttachToSession"));
    user->insert("lastName", QStringLiteral("lastName"));
    user->insert("password", QStringLiteral("password"));
    QVERIFY(EnginioTests::createObject(m_client, user));
    QVERIFY(!user->id().isEmpty());

    /* Login as new user */

    EnginioIdentityAuthOperation *op = new EnginioIdentityAuthOperation(m_client);
    QVERIFY2(op, "Operation creation failed");
    op->loginWithUsernameAndPassword(user->value("username").toString(),
                                     user->value("password").toString());

    QSignalSpy finishedSpy1(op, SIGNAL(finished()));
    QSignalSpy errorSpy1(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(sessionAuthSpy1.count(), 1);
    QCOMPARE(sessionTermSpy1.count(), 0);
    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 0);
    QVERIFY2(!m_client->sessionToken().isEmpty(), "Empty session token");
    QString sessionToken = m_client->sessionToken();

    delete op;

    /* Attach to existing session with session token */

    EnginioClient *secondClient = new EnginioClient(EnginioTests::TESTAPP_ID,
                                                    EnginioTests::TESTAPP_SECRET);
    QVERIFY2(secondClient, "Second client creation failed");
    secondClient->setApiUrl(EnginioTests::TESTAPP_URL);
    QSignalSpy sessionAuthSpy2(secondClient, SIGNAL(sessionAuthenticated()));
    QSignalSpy sessionTermSpy2(secondClient, SIGNAL(sessionTerminated()));

    op = new EnginioIdentityAuthOperation(secondClient);
    QVERIFY2(op, "Operation creation failed");
    op->attachToSessionWithToken(sessionToken);

    QSignalSpy finishedSpy2(op, SIGNAL(finished()));
    QSignalSpy errorSpy2(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy2.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(sessionAuthSpy2.count(), 1);
    QCOMPARE(sessionTermSpy2.count(), 0);
    QCOMPARE(finishedSpy2.count(), 1);
    QCOMPARE(errorSpy2.count(), 0);
    QCOMPARE(secondClient->sessionToken(), sessionToken);

    const EnginioJsonObject *loggedInUser = dynamic_cast<const EnginioJsonObject*>(
                op->loggedInUser());
    QVERIFY(loggedInUser);
    QCOMPARE(loggedInUser->value("username").toString(),
             user->value("username").toString());
    QCOMPARE(loggedInUser->value("firstName").toString(),
             user->value("firstName").toString());
    QCOMPARE(loggedInUser->value("lastName").toString(),
             user->value("lastName").toString());
    QVERIFY(loggedInUser->value("password").isUndefined());

    delete op;
    delete secondClient;
    delete user;
}

void AuthenticationTest::testCustomUserProperties()
{
    QVERIFY2(m_client, "Null client");
    QSignalSpy sessionAuthSpy(m_client, SIGNAL(sessionAuthenticated()));
    QSignalSpy sessionTermSpy(m_client, SIGNAL(sessionTerminated()));

    /* Create user object with custom fields */

    EnginioJsonObject *user = new EnginioJsonObject("users");
    user->insert("username", EnginioTests::randomString(15));
    user->insert("firstName", QStringLiteral("firstName_CustomUser"));
    user->insert("lastName", QStringLiteral("lastName"));
    user->insert("password", QStringLiteral("password"));
    user->insert("stringValue", QStringLiteral("CustomUserTag"));
    QVERIFY(EnginioTests::createObject(m_client, user));
    QVERIFY(!user->id().isEmpty());

    /* Login as custom user */

    EnginioIdentityAuthOperation *op = new EnginioIdentityAuthOperation(m_client);
    QVERIFY2(op, "Operation creation failed");
    op->loginWithUsernameAndPassword(user->value("username").toString(),
                                     user->value("password").toString());

    QSignalSpy finishedSpy1(op, SIGNAL(finished()));
    QSignalSpy errorSpy1(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(sessionAuthSpy.count(), 1);
    QCOMPARE(sessionTermSpy.count(), 0);
    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 0);
    QVERIFY2(!m_client->sessionToken().isEmpty(), "Empty session token");

    delete op;

    /* Create a new object */

    EnginioJsonObject *object = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    object->insert("stringValue", EnginioTests::randomString(5));
    QVERIFY(EnginioTests::createObject(m_client, object));
    QVERIFY(!object->id().isEmpty());
    QVERIFY(!object->value("createdAt").toString().isEmpty());
    QVERIFY(!object->value("updatedAt").toString().isEmpty());

    QJsonObject creator = object->value("creator").toObject();
    QVERIFY2(!creator.isEmpty(), "Object creator not found");
    QCOMPARE(creator.value("id").toString(), user->id());
    QCOMPARE(creator.value("objectType").toString(), QString("users"));
    QVERIFY(creator.value("stringValue").isUndefined());

    QJsonObject updater = object->value("updater").toObject();
    QVERIFY2(!updater.isEmpty(), "Object updater not found");
    QCOMPARE(updater.value("id").toString(), user->id());
    QCOMPARE(updater.value("objectType").toString(), QString("users"));
    QVERIFY(updater.value("stringValue").isUndefined());


    /* Fetch object and include creator & updater as full Users */

    EnginioObjectOperation *objOp = new EnginioObjectOperation(m_client);

    EnginioJsonObject *readObject = 0;
    readObject = dynamic_cast<EnginioJsonObject*>(
                objOp->read(object->id(), object->objectType()));
    QVERIFY2(readObject, "Object creation failed");
    objOp->setRequestParam("include", "{\"creator\": {}, \"updater\": {}}");

    QSignalSpy finishedSpy2(objOp, SIGNAL(finished()));
    QSignalSpy errorSpy2(objOp, SIGNAL(error(EnginioError*)));

    objOp->execute();

    QVERIFY2(finishedSpy2.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(finishedSpy2.count(), 1);
    QCOMPARE(errorSpy2.count(), 0);

    QVERIFY(!readObject->id().isEmpty());
    QVERIFY(!readObject->value("createdAt").toString().isEmpty());
    QVERIFY(!readObject->value("updatedAt").toString().isEmpty());

    creator = readObject->value("creator").toObject();
    QVERIFY2(!creator.isEmpty(), "Object creator not found");
    QCOMPARE(creator.value("id").toString(), user->id());
    QCOMPARE(creator.value("objectType").toString(), QString("users"));
    QCOMPARE(creator.value("stringValue").toString(),
             user->value("stringValue").toString());
    QCOMPARE(creator.value("username").toString(),
             user->value("username").toString());
    QCOMPARE(creator.value("firstName").toString(),
             user->value("firstName").toString());
    QCOMPARE(creator.value("lastName").toString(),
             user->value("lastName").toString());
    QVERIFY(creator.value("password").isUndefined());

    updater = readObject->value("updater").toObject();
    QVERIFY2(!updater.isEmpty(), "Object updater not found");
    QCOMPARE(updater.value("id").toString(), user->id());
    QCOMPARE(updater.value("objectType").toString(), QString("users"));
    QCOMPARE(updater.value("stringValue").toString(),
             user->value("stringValue").toString());
    QCOMPARE(updater.value("username").toString(),
             user->value("username").toString());
    QCOMPARE(updater.value("firstName").toString(),
             user->value("firstName").toString());
    QCOMPARE(updater.value("lastName").toString(),
             user->value("lastName").toString());
    QVERIFY(updater.value("password").isUndefined());

    delete objOp;
    delete readObject;
    delete object;
    delete user;
}

QTEST_MAIN(AuthenticationTest)

#include "tst_authenticationtest.moc"
