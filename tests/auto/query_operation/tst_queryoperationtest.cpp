#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include "../common/common.h"
#include "enginioclient.h"
#include "enginioerror.h"
#include "enginiojsonobject.h"
#include "enginioobjectmodel.h"
#include "enginioobjectoperation.h"
#include "enginioqueryoperation.h"

class QueryOperationTest : public QObject
{
    Q_OBJECT

public:
    QueryOperationTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testMissingType();
    void testQueryWithType_data();
    void testQueryWithType();
    void testQueryWithQuery();
    void testResultList();
    void testSearchSingleType();
    void testSearchMultipleTypes();

private:
    QPointer<EnginioClient> m_client;
    QPointer<EnginioObjectModel> m_model;
};


QueryOperationTest::QueryOperationTest()
{
    qsrand((uint)QTime::currentTime().msec());
    qRegisterMetaType<EnginioError*>(); // required by QSignalSpy
}

void QueryOperationTest::initTestCase()
{
    m_client = new EnginioClient(EnginioTests::TESTAPP_ID,
                                 EnginioTests::TESTAPP_SECRET,
                                 this);
    QVERIFY2(m_client, "Client creation failed");
    m_client->setApiUrl(EnginioTests::TESTAPP_URL);

    m_model = new EnginioObjectModel(this);
    QVERIFY2(m_model, "Model creation failed");
    QVERIFY(m_model->rowCount() == 0);

    // create TEST_OBJECT_TYPE object with intValue < 0
    EnginioJsonObject *object = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    object->insert("stringValue", QString("First query test object #%1").arg(qrand()));
    int randomNum = qrand() % 1000000 + 1;
    object->insert("intValue", -randomNum);
    QVERIFY(EnginioTests::createObject(m_client, object, m_model));
    QVERIFY(!object->id().isEmpty());
    delete object;

    // create TEST_OBJECT_TYPE object with intValue > 0
    object = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    object->insert("stringValue", QString("Second query test object #%1").arg(qrand()));
    randomNum = qrand() % 1000000 + 1;
    object->insert("intValue", randomNum);
    QVERIFY(EnginioTests::createObject(m_client, object, m_model));
    QVERIFY(!object->id().isEmpty());
    delete object;

    // create CUSTOM_OBJECT_TYPE object with intValue > 0
    object = new EnginioJsonObject(EnginioTests::CUSTOM_OBJECT_TYPE);
    object->insert("stringValue", QString("Query custom object #%1").arg(qrand()));
    randomNum = qrand() % 1000000 + 1;
    object->insert("intValue", randomNum);
    QVERIFY(EnginioTests::createObject(m_client, object, m_model));
    QVERIFY(!object->id().isEmpty());
    delete object;

    // create user object
    EnginioJsonObject *user = new EnginioJsonObject("users");
    user->insert("username", EnginioTests::randomString(15));
    user->insert("firstName", QStringLiteral("firstName_testQueryOperation"));
    user->insert("lastName", QStringLiteral("lastName_testQueryOperation"));
    user->insert("password", QStringLiteral("pa$$word"));
    QVERIFY(EnginioTests::createObject(m_client, user));
    QVERIFY(!user->id().isEmpty());
    delete user;
}

void QueryOperationTest::cleanupTestCase()
{
}

/*!
 * Try to fetch objects without specifying object type. This should emit error
 * signal as soon as operation is executed.
 */
void QueryOperationTest::testMissingType()
{
    QVERIFY2(m_client, "Null client");

    EnginioQueryOperation *op = new EnginioQueryOperation(m_client);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(op->error()->error(), EnginioError::RequestError);

    delete op;
}

void QueryOperationTest::testQueryWithType_data()
{
    QTest::addColumn<QString>("objectType");

    QTest::newRow("test object") << EnginioTests::TEST_OBJECT_TYPE;
    QTest::newRow("user") << QStringLiteral("users");
}

/*!
 * Fetch all objects matching test object type.
 */
void QueryOperationTest::testQueryWithType()
{
    QFETCH(QString, objectType);

    QVERIFY2(m_client, "Null client");

    EnginioObjectModel *model = new EnginioObjectModel();

    EnginioQueryOperation *op = new EnginioQueryOperation(m_client, model);
    op->setObjectType(objectType);
    QCOMPARE(model->rowCount(), 0);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));
    QSignalSpy insertSpy(model, SIGNAL(rowsInserted(QModelIndex,int,int)));

    op->execute();

    QVERIFY2(finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation timeout");
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(insertSpy.count(), 1);

    if (objectType == QStringLiteral("users"))
        QVERIFY2(model->rowCount() > 1, "No users added to model");
    else
        QVERIFY2(model->rowCount() > 2, "No objects added to model");

    for (int i = 0; i < model->rowCount(); i++) {
        EnginioJsonObject *obj = dynamic_cast<EnginioJsonObject*>(
                    model->getObject(model->index(i)));
        QVERIFY2(obj, "Invalid object in model");
        QCOMPARE(obj->objectType(), objectType);
    }

    QList<EnginioAbstractObject*> results = op->takeResults();
    QVERIFY(results.isEmpty());

    delete op;
    delete model;
}

/*!
 * Fetch test objects where intValue is less than zero. In initTestCase we make
 * sure that there is at least one such object in backend.
 */
void QueryOperationTest::testQueryWithQuery()
{
    QVERIFY2(m_client, "Null client");

    EnginioObjectModel *model = new EnginioObjectModel();
    EnginioQueryOperation *op = new EnginioQueryOperation(m_client);
    op->setObjectTypes(QStringList() << EnginioTests::TEST_OBJECT_TYPE);
    op->setModel(model);
    op->setRequestParam("q", "{\"intValue\": {\"$lt\": 0}}");
    QCOMPARE(model->rowCount(), 0);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));
    QSignalSpy insertSpy(model, SIGNAL(rowsInserted(QModelIndex,int,int)));

    op->execute();

    QVERIFY2(finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation timeout");
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(insertSpy.count(), 1);
    QVERIFY2(model->rowCount() > 0, "No objects added to model");

    for (int i = 0; i < model->rowCount(); i++) {
        EnginioJsonObject *obj = dynamic_cast<EnginioJsonObject*>(
                    model->getObject(model->index(i)));
        QVERIFY2(obj, "Invalid object in model");
        QVERIFY2(obj->value("intValue").toDouble() < 0,
                 "Invalid value in result object");
    }

    delete op;
    delete model;
}

/*!
 * Fetch test objects where intValue is less than zero without using model.
 */
void QueryOperationTest::testResultList()
{
    QVERIFY2(m_client, "Null client");

    EnginioQueryOperation *op = new EnginioQueryOperation(m_client);
    op->addObjectType(EnginioTests::TEST_OBJECT_TYPE);
    op->setRequestParam("q", "{\"intValue\": {\"$lt\": 0}}");

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation timeout");
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);

    QList<EnginioAbstractObject*> results = op->takeResults();
    QVERIFY2(results.size() > 0,
             QString("Too few result objects: %1").arg(results.size()).toLatin1());
    for (int i = 0; i < results.size(); i++) {
        EnginioJsonObject *object = dynamic_cast<EnginioJsonObject*>(
                    results.at(i));
        QVERIFY(object);
        QVERIFY2(object->value("intValue").toDouble() < 0,
                 "Invalid value in result object");
    }

    delete op;
    while (!results.isEmpty())
        delete results.takeFirst();
}

/*!
 * Fetch one TEST_OBJECT_TYPE object with SEARCH_STRING in stringValue
 * property.
 */
void QueryOperationTest::testSearchSingleType()
{
    // Use string that is not found in first TEST_OBJECT_TYPE object. That way
    // we can detect if object query API is used instead of seach API.
    const QString SEARCH_STRING("Second");

    QVERIFY2(m_client, "Null client");

    EnginioObjectModel *model = new EnginioObjectModel();
    EnginioQueryOperation *op = new EnginioQueryOperation(m_client);
    op->addObjectType(EnginioTests::TEST_OBJECT_TYPE);
    op->setRequestParam("search", QString("{\"phrase\":\"%1\",\"properties\":[\"stringValue\"]}").arg(SEARCH_STRING));
    op->setRequestParam("limit", "1");
    op->setModel(model);
    QCOMPARE(model->rowCount(), 0);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));
    QSignalSpy insertSpy(model, SIGNAL(rowsInserted(QModelIndex,int,int)));

    op->execute();

    QVERIFY2(finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation timeout");
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(insertSpy.count(), 1);
    QCOMPARE(model->rowCount(), 1);

    EnginioJsonObject *obj = dynamic_cast<EnginioJsonObject*>(
                model->getObject(model->index(0)));
    QVERIFY2(obj, "Invalid object in model");
    QCOMPARE(obj->value("objectType").toString(),
             EnginioTests::TEST_OBJECT_TYPE);
    QString stringValue = obj->value("stringValue").toString();
    QVERIFY2(stringValue.contains(SEARCH_STRING), "Invalid search result");

    delete op;
    delete model;
}

/*!
 * Fetch all TEST_OBJECT_TYPE and CUSTOM_OBJECT_TYPE obejcts with SEARCH_STRING
 * in stringValue property.
 */
void QueryOperationTest::testSearchMultipleTypes()
{
    const QString SEARCH_STRING("Query");

    QVERIFY2(m_client, "Null client");

    EnginioObjectModel *model = new EnginioObjectModel();
    EnginioQueryOperation *op = new EnginioQueryOperation(m_client);
    op->setObjectTypes(QStringList() <<
                       EnginioTests::TEST_OBJECT_TYPE <<
                       EnginioTests::CUSTOM_OBJECT_TYPE);
    op->setRequestParam("search", QString("{\"phrase\":\"%1\",\"properties\":[\"stringValue\"]}").arg(SEARCH_STRING));
    op->setModel(model);
    QCOMPARE(model->rowCount(), 0);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));
    QSignalSpy insertSpy(model, SIGNAL(rowsInserted(QModelIndex,int,int)));

    op->execute();

    QVERIFY2(finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation timeout");
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(insertSpy.count(), 1);
    QVERIFY2(model->rowCount() > 0, "No objects added to model");

    int testObjectsFound = 0;
    int customObjectsFound = 0;
    for (int i = 0; i < model->rowCount(); i++) {
        EnginioJsonObject *obj = dynamic_cast<EnginioJsonObject*>(
                    model->getObject(model->index(i)));
        QVERIFY2(obj, "Invalid object in model");
        QString type = obj->value("objectType").toString();
        if (type == EnginioTests::TEST_OBJECT_TYPE)
            testObjectsFound++;
        else if (type == EnginioTests::CUSTOM_OBJECT_TYPE)
            customObjectsFound++;
        else
            QFAIL("Invalid object type");

        QString stringValue = obj->value("stringValue").toString();
        QVERIFY2(stringValue.contains(SEARCH_STRING, Qt::CaseInsensitive),
                 "Invalid search result");
    }
    QVERIFY2(testObjectsFound > 1, "Failed to find all test objects");
    QVERIFY2(customObjectsFound > 0, "Failed to find all custom objects");

    delete op;
    delete model;
}

QTEST_MAIN(QueryOperationTest)

#include "tst_queryoperationtest.moc"
