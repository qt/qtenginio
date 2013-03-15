#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include "../common/common.h"
#include "enginioacloperation.h"
#include "enginioclient.h"
#include "enginioerror.h"
#include "enginiojsonobject.h"
#include "enginioobjectmodel.h"
#include "enginioobjectoperation.h"

class AclOperationTest : public QObject
{
    Q_OBJECT
    
public:
    AclOperationTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testReadObjectPermissions();
    void testAddObjectPermissions();
    void testDeleteObjectPermissions();
    void testUpdateObjectPermissions();

private:
    EnginioClient *m_client;
};

AclOperationTest::AclOperationTest()
{
    qsrand((uint)QTime::currentTime().msec());
    qRegisterMetaType<EnginioError*>(); // required by QSignalSpy
    qRegisterMetaType<EnginioOperation::State>("State");
}

void AclOperationTest::initTestCase()
{
    m_client = new EnginioClient(EnginioTests::TESTAPP_ID,
                                 EnginioTests::TESTAPP_SECRET,
                                 this);
    QVERIFY2(m_client, "Client creation failed");
    m_client->setApiUrl(EnginioTests::TESTAPP_URL);
}

void AclOperationTest::cleanupTestCase()
{
}

void AclOperationTest::testReadObjectPermissions()
{
    QVERIFY2(m_client, "Null client");

    /* Create user object and login */

    EnginioJsonObject *user = new EnginioJsonObject("users");
    user->insert("username", EnginioTests::randomString(15));
    user->insert("firstName", QStringLiteral("firstName_ReadACL"));
    user->insert("lastName", QStringLiteral("lastName"));
    user->insert("password", QStringLiteral("password"));
    QVERIFY(EnginioTests::createObject(m_client, user));
    QVERIFY(!user->id().isEmpty());
    QVERIFY(EnginioTests::login(m_client,
                                user->value("username").toString(),
                                user->value("password").toString()));

    /* Create new object */

    EnginioJsonObject *object = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    object->insert("stringValue", QString("testReadObjectPermissions #%1").arg(qrand()));
    object->insert("intValue", qrand());
    QVERIFY(EnginioTests::createObject(m_client, object));
    QVERIFY(!object->id().isEmpty());

    /* Get object ACL */

    EnginioAclOperation *op = new EnginioAclOperation(m_client);
    op->setObject(qMakePair(object->id(), object->objectType()));
    op->readPermissions();

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY(finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT));
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);

    QSharedPointer<EnginioAcl> resultAcl = op->resultAcl();
    QVERIFY(!resultAcl.isNull());

    delete op;

    QPair<QString, QString> everybody = qMakePair(QStringLiteral("*"),
                                                  QStringLiteral("aclSubject"));
    QVERIFY(resultAcl->isSubjectHavingPermission(everybody,
                                                 EnginioAcl::ReadPermission));
    QVERIFY(resultAcl->isSubjectHavingPermission(everybody,
                                                 EnginioAcl::UpdatePermission));
    QVERIFY(resultAcl->isSubjectHavingPermission(everybody,
                                                 EnginioAcl::DeletePermission));
    QVERIFY(resultAcl->isSubjectHavingPermission(qMakePair(user->id(),
                                                           user->objectType()),
                                                 EnginioAcl::AdminPermission));
    resultAcl.clear();
    delete user;
    delete object;
}

void AclOperationTest::testAddObjectPermissions()
{
    QVERIFY2(m_client, "Null client");

    /* Create user1 and user2 and login user1 */

    EnginioJsonObject *user1 = new EnginioJsonObject("users");
    user1->insert("username", EnginioTests::randomString(15));
    user1->insert("firstName", QStringLiteral("firstName_AddACL1"));
    user1->insert("lastName", QStringLiteral("lastName"));
    user1->insert("password", QStringLiteral("password"));
    QVERIFY(EnginioTests::createObject(m_client, user1));
    QVERIFY(!user1->id().isEmpty());

    EnginioJsonObject *user2 = new EnginioJsonObject("users");
    user2->insert("username", EnginioTests::randomString(15));
    user2->insert("firstName", QStringLiteral("firstName_AddACL2"));
    user2->insert("lastName", QStringLiteral("lastName"));
    user2->insert("password", QStringLiteral("password"));
    QVERIFY(EnginioTests::createObject(m_client, user2));
    QVERIFY(!user2->id().isEmpty());

    QVERIFY(EnginioTests::login(m_client,
                                user1->value("username").toString(),
                                user1->value("password").toString()));

    /* Create new object */

    EnginioJsonObject *object = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    object->insert("stringValue", QString("testAddObjectPermissions #%1").arg(qrand()));
    object->insert("intValue", qrand());
    QVERIFY(EnginioTests::createObject(m_client, object));
    QVERIFY(!object->id().isEmpty());

    /* Add admin access to object for user2 */

    EnginioAclOperation *op = new EnginioAclOperation(m_client);
    op->setObject(qMakePair(object->id(), object->objectType()));
    op->grantPermission(qMakePair(user2->id(), user2->objectType()),
                        EnginioAcl::AdminPermission);

    QSignalSpy finishedSpy1(op, SIGNAL(finished()));
    QSignalSpy errorSpy1(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY(finishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT));
    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 0);
    delete op;

    /* Logout user1 and login user 2 */

    QVERIFY(EnginioTests::logout(m_client));
    QVERIFY(EnginioTests::login(m_client,
                                user2->value("username").toString(),
                                user2->value("password").toString()));

    /* Read object permissions as user2 */

    op = new EnginioAclOperation(m_client);
    op->setObject(qMakePair(object->id(), object->objectType()));
    op->readPermissions();

    QSignalSpy finishedSpy2(op, SIGNAL(finished()));
    QSignalSpy errorSpy2(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY(finishedSpy2.wait(EnginioTests::NETWORK_TIMEOUT));
    QCOMPARE(finishedSpy2.count(), 1);
    QCOMPARE(errorSpy2.count(), 0);

    QSharedPointer<EnginioAcl> resultAcl = op->resultAcl();
    QVERIFY(!resultAcl.isNull());
    QVERIFY(resultAcl->isSubjectHavingPermission(qMakePair(user1->id(),
                                                           user1->objectType()),
                                                 EnginioAcl::AdminPermission));
    QVERIFY(resultAcl->isSubjectHavingPermission(qMakePair(user2->id(),
                                                           user2->objectType()),
                                                 EnginioAcl::AdminPermission));
    resultAcl.clear();
    delete op;
    delete object;
    delete user1;
    delete user2;
}

void AclOperationTest::testDeleteObjectPermissions()
{
    QVERIFY2(m_client, "Null client");

    /* Create user1 and user2 and login user1 */

    EnginioJsonObject *user1 = new EnginioJsonObject("users");
    user1->insert("username", EnginioTests::randomString(15));
    user1->insert("firstName", QStringLiteral("firstName_DeleteACL1"));
    user1->insert("lastName", QStringLiteral("lastName"));
    user1->insert("password", QStringLiteral("password"));
    QVERIFY(EnginioTests::createObject(m_client, user1));
    QVERIFY(!user1->id().isEmpty());

    EnginioJsonObject *user2 = new EnginioJsonObject("users");
    user2->insert("username", EnginioTests::randomString(15));
    user2->insert("firstName", QStringLiteral("firstName_DeleteACL2"));
    user2->insert("lastName", QStringLiteral("lastName"));
    user2->insert("password", QStringLiteral("password"));
    QVERIFY(EnginioTests::createObject(m_client, user2));
    QVERIFY(!user2->id().isEmpty());

    QVERIFY(EnginioTests::login(m_client,
                                user1->value("username").toString(),
                                user1->value("password").toString()));

    /* Create new object */

    EnginioJsonObject *object = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    object->insert("stringValue", QString("testDeleteObjectPermissions #%1").arg(qrand()));
    object->insert("intValue", qrand());
    QVERIFY(EnginioTests::createObject(m_client, object));
    QVERIFY(!object->id().isEmpty());

    /* Remove global read access from object */

    EnginioAclOperation *op = new EnginioAclOperation(m_client);
    op->setObject(qMakePair(object->id(), object->objectType()));
    op->withdrawPermission(qMakePair(QStringLiteral("*"),
                                     QStringLiteral("aclSubject")),
                           EnginioAcl::ReadPermission);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY(finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT));
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    delete op;

    /* Logout user1 and login user 2 */

    QVERIFY(EnginioTests::logout(m_client));
    QVERIFY(EnginioTests::login(m_client,
                                user2->value("username").toString(),
                                user2->value("password").toString()));

    /* Try to read object as user2 */

    EnginioJsonObject *readObject2 = dynamic_cast<EnginioJsonObject*>(
                EnginioTests::readObject(m_client, object->id(),
                                         object->objectType()));
    QVERIFY(!readObject2);

    delete object;
    delete user1;
    delete user2;
}

void AclOperationTest::testUpdateObjectPermissions()
{
    const QString FAKE_USER_ID("111222333444555666777888");

    QVERIFY2(m_client, "Null client");

    /* Create user and login */

    EnginioJsonObject *user = new EnginioJsonObject("users");
    user->insert("username", EnginioTests::randomString(15));
    user->insert("firstName", QStringLiteral("firstName_UpdateACL"));
    user->insert("lastName", QStringLiteral("lastName"));
    user->insert("password", QStringLiteral("password"));
    QVERIFY(EnginioTests::createObject(m_client, user));
    QVERIFY(!user->id().isEmpty());
    QVERIFY(EnginioTests::login(m_client,
                                user->value("username").toString(),
                                user->value("password").toString()));

    /* Create new object */

    EnginioJsonObject *object = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    object->insert("stringValue", QString("testUpdateObjectPermissions #%1").arg(qrand()));
    object->insert("intValue", qrand());
    QVERIFY(EnginioTests::createObject(m_client, object));
    QVERIFY(!object->id().isEmpty());

    /* Set object permissions */

    QSharedPointer<EnginioAcl> acl = QSharedPointer<EnginioAcl>(new EnginioAcl());
    acl->addSubjectForPermission(qMakePair(user->id(), user->objectType()),
                                 EnginioAcl::AdminPermission);
    acl->addSubjectForPermission(qMakePair(FAKE_USER_ID, QStringLiteral("users")),
                                 EnginioAcl::ReadPermission);

    EnginioAclOperation *op = new EnginioAclOperation(m_client);
    op->setObject(qMakePair(object->id(), object->objectType()));
    op->setPermissions(acl);
    acl.clear();

    QSignalSpy finishedSpy1(op, SIGNAL(finished()));
    QSignalSpy errorSpy1(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY(finishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT));
    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 0);
    delete op;

    /* Read object permissions */

    op = new EnginioAclOperation(m_client);
    op->setObject(qMakePair(object->id(), object->objectType()));
    op->readPermissions();

    QSignalSpy finishedSpy2(op, SIGNAL(finished()));
    QSignalSpy errorSpy2(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY(finishedSpy2.wait(EnginioTests::NETWORK_TIMEOUT));
    QCOMPARE(finishedSpy2.count(), 1);
    QCOMPARE(errorSpy2.count(), 0);

    QSharedPointer<EnginioAcl> resultAcl = op->resultAcl();
    QVERIFY(!resultAcl.isNull());
    QVERIFY(resultAcl->isSubjectHavingPermission(qMakePair(user->id(),
                                                           user->objectType()),
                                                 EnginioAcl::AdminPermission));
    QVERIFY(resultAcl->isSubjectHavingPermission(qMakePair(FAKE_USER_ID,
                                                           QStringLiteral("users")),
                                                 EnginioAcl::ReadPermission));
    QVERIFY(!resultAcl->isSubjectHavingPermission(qMakePair(FAKE_USER_ID,
                                                           QStringLiteral("users")),
                                                 EnginioAcl::UpdatePermission));
    resultAcl.clear();
    delete op;

    delete object;
    delete user;
}

QTEST_MAIN(AclOperationTest)

#include "tst_acloperationtest.moc"
