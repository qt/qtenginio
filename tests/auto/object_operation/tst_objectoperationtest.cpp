#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <QDebug>

#include "../common/common.h"
#include "enginioclient.h"
#include "enginioerror.h"
#include "enginiojsonobject.h"
#include "enginioobjectmodel.h"
#include "enginioobjectoperation.h"
#include "selflinkedobject.h"
#include "testobjectfactory.h"

class ObjectOperationTest : public QObject
{
    Q_OBJECT

public:
    ObjectOperationTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCreateInvalid();
    void testCreateWithMissingType();
    void testCreate();
    void testRead();
    void testUpdate();
    void testRemove();
    void testCreateAndCancel();
    void testCustomObjectWithRefs();
    void testObjectRefsChain();
    void testSelfLinkingObjectRefs();
    void testDeleteProperty();

private:
    QPointer<EnginioClient> m_client;
    QPointer<EnginioObjectModel> m_model;
};

ObjectOperationTest::ObjectOperationTest()
{
    qsrand((uint)QTime::currentTime().msec());
    qRegisterMetaType<EnginioError*>(); // required by QSignalSpy
    qRegisterMetaType<EnginioOperation::State>("State");
}

void ObjectOperationTest::initTestCase()
{
    m_client = new EnginioClient(EnginioTests::TESTAPP_ID,
                                 EnginioTests::TESTAPP_SECRET,
                                 this);
    QVERIFY2(m_client, "Client creation failed");
    m_client->setApiUrl(EnginioTests::TESTAPP_URL);

    m_model = new EnginioObjectModel(this);
}

void ObjectOperationTest::cleanupTestCase()
{
}

/*!
 * Try to execute operation without calling CRUD functions first. Error signal
 * should be emitted immediatelly after execute() because operation type is
 * unknown.
 */
void ObjectOperationTest::testCreateInvalid()
{
    QVERIFY2(m_client, "Null client");

    EnginioObjectOperation *op = new EnginioObjectOperation(m_client);
    QVERIFY2(op, "Operation creation failed");

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(op->error()->error(), EnginioError::RequestError);

    delete op;
}

/*!
 * Try to create operation with invalid object (missing objectType) Error signal
 * should be emitted immediatelly after execute() because object type is unknown
 * and thus request URL cannot be created.
 */
void ObjectOperationTest::testCreateWithMissingType()
{
    QVERIFY2(m_client, "Null client");

    EnginioJsonObject *newObject = new EnginioJsonObject();
    QVERIFY2(newObject, "Object creation failed");
    newObject->insert("stringValue", EnginioTests::randomString(10));
    newObject->insert("intValue", 34);
    QVERIFY2(newObject->id().isEmpty(), "New object's ID not empty");

    EnginioObjectOperation *op = new EnginioObjectOperation(m_client);
    op->create(newObject);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(op->error()->error(), EnginioError::RequestError);
    QVERIFY2(newObject->id().isEmpty(), "Object ID was set");

    delete op;
    delete newObject;
}

/*!
 * Create basic object in backend
 */
void ObjectOperationTest::testCreate()
{
    QVERIFY2(m_client, "Null client");
    QVERIFY2(m_model, "Null model");

    QDateTime before = QDateTime::currentDateTimeUtc();

    QString stringValue = QString("testCreate #%1").arg(qrand());
    int intValue = 987654321;

    EnginioJsonObject *newObject = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    QVERIFY2(newObject, "Object creation failed");
    newObject->insert("stringValue", stringValue);
    newObject->insert("intValue", intValue);
    QVERIFY2(newObject->id().isEmpty(), "New object's ID not empty");

    int modelRows = m_model->rowCount();

    EnginioObjectOperation *op = new EnginioObjectOperation(m_client, m_model);
    op->create(newObject);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    QVERIFY2(!newObject->id().isEmpty(), "Object ID not set");
    QVERIFY2(!newObject->value("createdAt").isUndefined(), "Create time undefined");
    QVERIFY2(!newObject->value("updatedAt").isUndefined(), "Update time undefined");
    QCOMPARE(newObject->value("createdAt").toString(),
             newObject->value("updatedAt").toString());
    QCOMPARE(newObject->value("stringValue").toString(), stringValue);
    QCOMPARE((int)newObject->value("intValue").toDouble(), intValue);
    QCOMPARE(m_model->rowCount(), modelRows + 1);

    QDateTime after = QDateTime::currentDateTimeUtc();
    QDateTime createTime = QDateTime::fromString(newObject->value("createdAt").toString(),
                                                 EnginioAbstractObject::timeFormat());
    createTime.setTimeSpec(Qt::UTC);
    QDateTime updateTime = QDateTime::fromString(newObject->value("updatedAt").toString(),
                                                 EnginioAbstractObject::timeFormat());
    updateTime.setTimeSpec(Qt::UTC);

    qDebug() << "before:" << before.toString(EnginioAbstractObject::timeFormat());
    qDebug() << "create:" << createTime.toString(EnginioAbstractObject::timeFormat());
    qDebug() << "update:" << updateTime.toString(EnginioAbstractObject::timeFormat());
    qDebug() << "after:" << after.toString(EnginioAbstractObject::timeFormat());

    // Comparing local time and server time
//    QVERIFY(createTime > before);
//    QVERIFY(createTime < after);
//    QVERIFY(updateTime > before);
//    QVERIFY(updateTime < after);

    delete op;
    delete newObject;
}

/*!
 * Read object data from backend
 */
void ObjectOperationTest::testRead()
{
    QVERIFY2(m_client, "Null client");
    QVERIFY2(m_model, "Null model");

    /* Create new object in backend */

    EnginioJsonObject *originalObject = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    originalObject->insert("stringValue", EnginioTests::randomString(10));
    originalObject->insert("intValue", qrand());
    QVERIFY(EnginioTests::createObject(m_client, originalObject));
    QVERIFY(!originalObject->id().isEmpty());

    /* Add object to m_model with same ID */

    EnginioJsonObject *modelObject = new EnginioJsonObject();
    modelObject->insert("id", originalObject->id());
    QVERIFY(modelObject->value("stringValue").isUndefined());
    QVERIFY(modelObject->value("intValue").isUndefined());

    QList<EnginioAbstractObject*>objects;
    objects << modelObject;
    m_model->addToModel(objects, 0);
    int modelRowCountBefore = m_model->rowCount();
    QVERIFY(modelRowCountBefore >= 1);
    QVERIFY(m_model->indexFromId(originalObject->id()).isValid());

    /* Read object data from backend */

    EnginioJsonObject *readObject = dynamic_cast<EnginioJsonObject*>(
                EnginioTests::readObject(m_client,
                                         originalObject->id(),
                                         originalObject->objectType(),
                                         m_model));
    QVERIFY2(readObject, "Failed to read object");

    /* Check that object returned from read() was updated correctly */

    QCOMPARE(readObject->value("stringValue").toString(),
             originalObject->value("stringValue").toString());
    QCOMPARE((int)readObject->value("intValue").toDouble(),
             (int)originalObject->value("intValue").toDouble());

    /* Check that object in model was updated correctly */

    QCOMPARE(m_model->rowCount(), modelRowCountBefore);
    QModelIndex index = m_model->indexFromId(originalObject->id());
    QVERIFY2(index.isValid(), "Can't find object from model");
    EnginioJsonObject *modelObject2 = dynamic_cast<EnginioJsonObject*>(
                m_model->getObject(index));
    QCOMPARE(modelObject2->value("stringValue").toString(),
             originalObject->value("stringValue").toString());
    QCOMPARE((int)modelObject2->value("intValue").toDouble(),
             (int)originalObject->value("intValue").toDouble());

    delete originalObject;
    delete readObject;
}

/*!
 * Update object on backend
 */
void ObjectOperationTest::testUpdate()
{
    QVERIFY2(m_client, "Null client");
    QVERIFY2(m_model, "Null model");

    QDateTime beforeCreate = QDateTime::currentDateTimeUtc();

    /* Create new object in backend and add it to model */

    EnginioJsonObject *originalObject = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    originalObject->insert("stringValue", EnginioTests::randomString(10));
    originalObject->insert("intValue", qrand());
    QVERIFY(EnginioTests::createObject(m_client, originalObject, m_model));
    QVERIFY(!originalObject->id().isEmpty());
    int modelRowCountBefore = m_model->rowCount();

    /* Update object data */

    QDateTime beforeUpdate = QDateTime::currentDateTimeUtc();

    QString newString(originalObject->value("stringValue").toString());
    newString.append(" (updated)");

    EnginioJsonObject *updateObject = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    QVERIFY2(updateObject, "Object creation failed");
    updateObject->insert("id", originalObject->id());
    updateObject->insert("stringValue", newString);

    EnginioObjectOperation *op = new EnginioObjectOperation(m_client);
    op->setModel(m_model);
    op->update(updateObject);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);

    /* Check that object used in update operation was updated correctly */

    QCOMPARE(updateObject->value("stringValue").toString(), newString);
    QCOMPARE((int)updateObject->value("intValue").toDouble(),
             (int)originalObject->value("intValue").toDouble());
    QVERIFY2(!updateObject->value("createdAt").isUndefined(), "Create time undefined");
    QVERIFY2(!updateObject->value("updatedAt").isUndefined(), "Update time undefined");
    QVERIFY(updateObject->value("createdAt").toString() !=
            updateObject->value("updatedAt").toString());

    /* Check that object in model was updated correctly */

    QCOMPARE(m_model->rowCount(), modelRowCountBefore);
    QModelIndex index = m_model->indexFromId(originalObject->id());
    QVERIFY2(index.isValid(), "Can't find object from model");
    EnginioJsonObject *modelObject2 = dynamic_cast<EnginioJsonObject*>(
                m_model->getObject(index));
    QCOMPARE(modelObject2->value("stringValue").toString(), newString);
    QCOMPARE((int)modelObject2->value("intValue").toDouble(),
             (int)originalObject->value("intValue").toDouble());

    /* Get updated object from backend */

    EnginioJsonObject *readObject = dynamic_cast<EnginioJsonObject*>(
                EnginioTests::readObject(m_client,
                                         originalObject->id(),
                                         originalObject->objectType()));
    QVERIFY2(readObject, "Failed to read object");
    QCOMPARE(modelObject2->value("stringValue").toString(), newString);
    QCOMPARE((int)modelObject2->value("intValue").toDouble(),
             (int)originalObject->value("intValue").toDouble());

    QDateTime afterUpdate = QDateTime::currentDateTimeUtc();
    QDateTime createTime = QDateTime::fromString(readObject->value("createdAt").toString(),
                                                 EnginioAbstractObject::timeFormat());
    createTime.setTimeSpec(Qt::UTC);
    QDateTime updateTime = QDateTime::fromString(readObject->value("updatedAt").toString(),
                                                 EnginioAbstractObject::timeFormat());
    updateTime.setTimeSpec(Qt::UTC);

    qDebug() << "beforeCreate:" << beforeCreate.toString(EnginioAbstractObject::timeFormat());
    qDebug() << "create:" << createTime.toString(EnginioAbstractObject::timeFormat());
    qDebug() << "beforeUpdate:" << beforeUpdate.toString(EnginioAbstractObject::timeFormat());
    qDebug() << "update:" << updateTime.toString(EnginioAbstractObject::timeFormat());
    qDebug() << "afterUpdate:" << afterUpdate.toString(EnginioAbstractObject::timeFormat());

    // Comparing local time and server time :(
//    QVERIFY(createTime > beforeCreate);
//    QVERIFY(createTime < beforeUpdate);
//    QVERIFY(updateTime > beforeUpdate);
//    QVERIFY(updateTime < afterUpdate);

    delete op;
    delete originalObject;
    delete updateObject;
    delete readObject;
}

/*!
 * Delete object from backend
 */
void ObjectOperationTest::testRemove()
{
    QVERIFY2(m_client, "Null client");
    QVERIFY2(m_model, "Null model");

    /* Create new object in backend and add it to model */

    EnginioJsonObject *originalObject = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    originalObject->insert("stringValue", EnginioTests::randomString(10));
    originalObject->insert("intValue", qrand());
    QVERIFY(EnginioTests::createObject(m_client, originalObject, m_model));
    QVERIFY(!originalObject->id().isEmpty());
    int modelRowCountBefore = m_model->rowCount();

    /* Delete object from backend */

    EnginioObjectOperation *op = new EnginioObjectOperation(m_client);
    op->setModel(m_model);
    op->remove(originalObject->id(), originalObject->objectType());

    QSignalSpy removeFinishedSpy(op, SIGNAL(finished()));
    QSignalSpy removeErrorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(removeFinishedSpy.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(removeFinishedSpy.count(), 1);
    QCOMPARE(removeErrorSpy.count(), 0);

    /* Check that object was removed from model */

    QCOMPARE(m_model->rowCount(), modelRowCountBefore - 1);
    QVERIFY(!m_model->indexFromId(originalObject->id()).isValid());

    /* Try to get deleted object from backend */

    EnginioJsonObject *readObject = dynamic_cast<EnginioJsonObject*>(
                EnginioTests::readObject(m_client,
                                         originalObject->id(),
                                         originalObject->objectType()));
    QVERIFY(!readObject);

    delete op;
    delete originalObject;
}

/*!
 * Execute object create operation and cancel it immediately.
 */
void ObjectOperationTest::testCreateAndCancel()
{
    QVERIFY2(m_client, "Null client");

    EnginioJsonObject newObject(EnginioTests::TEST_OBJECT_TYPE);
    newObject.insert("stringValue", EnginioTests::randomString(20));
    newObject.insert("intValue", qrand());

    EnginioObjectOperation *op = new EnginioObjectOperation(m_client);
    op->create(newObject);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));
    QSignalSpy stateSpy(op, SIGNAL(stateChanged(State)));

    op->execute();

    QCOMPARE(stateSpy.count(), 1);
    QCOMPARE(op->state(), EnginioOperation::StateExecuting);

    op->cancel();

    QVERIFY2(finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT),
             "finished signal was not emitted");
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(op->error()->networkError(), QNetworkReply::OperationCanceledError);
    QCOMPARE(op->state(), EnginioOperation::StateCanceled);
    QCOMPARE(stateSpy.count(), 2);
}

/*!
 * Create and read objects using user defined object class.
 */
void ObjectOperationTest::testCustomObjectWithRefs()
{
    QVERIFY2(m_client, "Null client");

    int factoryId = m_client->registerObjectFactory(new TestObjectFactory());
    QVERIFY2(factoryId > 0, "Invalid factory registration ID");

    // Create Object1

    SelfLinkedObject *object1 = new SelfLinkedObject(
                QString(), "object1_testCustomObjectWithRefs", 11111111);
    QVERIFY(EnginioTests::createObject(m_client, object1));
    QVERIFY2(!object1->id().isEmpty(), "Object ID not set");

    // Create Object2

    SelfLinkedObject *object2 = new SelfLinkedObject(
                QString(), "object2_testCustomObjectWithRefs", 22222222);
    QVERIFY(EnginioTests::createObject(m_client, object2));
    QVERIFY2(!object2->id().isEmpty(), "Object ID not set");

    // Create Object3 which is linked to Objects 1 and 2

    SelfLinkedObject *object3 = new SelfLinkedObject(
                QString(), "object3_testCustomObjectWithRefs", 33333333,
                object1, object2);
    QVERIFY(EnginioTests::createObject(m_client, object3));
    QVERIFY2(!object3->id().isEmpty(), "Object ID not set");
    QCOMPARE(object3->m_objectValue1, object1);
    QCOMPARE(object3->m_objectValue2, object2);

    // Delete Object1

    EnginioObjectOperation *op = new EnginioObjectOperation(m_client);
    op->remove(object1->id(), object1->objectType());

    QSignalSpy finishedSpy1(op, SIGNAL(finished()));
    QSignalSpy errorSpy1(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation 4 was too slow");
    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 0);

    delete op;

    // Read Object3 and include objectValue1 & objectValue2 as full objects

    QMap<QString, QString> requestParams;
    requestParams[QStringLiteral("include")] =
            QStringLiteral("{\"objectValue1\": {}, \"objectValue2\": {}}");
    SelfLinkedObject *object3Read = dynamic_cast<SelfLinkedObject*>(
                EnginioTests::readObject(m_client,
                                         object3->id(),
                                         object3->objectType(),
                                         0,
                                         requestParams));

    QVERIFY(object3Read);
    QCOMPARE(object3Read->objectType(),
             QStringLiteral("objects.SelfLinkedObject"));
    QCOMPARE(object3Read->id(), object3->id());
    QCOMPARE(object3Read->m_stringValue, object3->m_stringValue);
    QCOMPARE(object3Read->m_intValue, object3->m_intValue);
    QVERIFY(!object3Read->m_objectValue1);
    QVERIFY(object3Read->m_objectValue2);
    QCOMPARE(object3Read->m_objectValue2->objectType(),
             QStringLiteral("objects.SelfLinkedObject"));
    QCOMPARE(object3Read->m_objectValue2->id(), object2->id());
    QCOMPARE(object3Read->m_objectValue2->m_stringValue, object2->m_stringValue);
    QCOMPARE(object3Read->m_objectValue2->m_intValue, object2->m_intValue);

    delete object3Read;
    delete object1;
    delete object2;
    delete object3;

    m_client->unregisterObjectFactory(factoryId);
}

/*!
 * Create and read objects containing references to other enginio objects.
 */
void ObjectOperationTest::testObjectRefsChain()
{
    QVERIFY2(m_client, "Null client");

    int factoryId = m_client->registerObjectFactory(new TestObjectFactory());
    QVERIFY2(factoryId > 0, "Invalid factory registration ID");

    /* Create object chain: object1 <= object2 <= object3 */

    SelfLinkedObject *object1 = new SelfLinkedObject(
                QString(), "object1_testObjectRefsChain", 11111111);
    QVERIFY(EnginioTests::createObject(m_client, object1));
    QVERIFY2(!object1->id().isEmpty(), "Object ID not set");

    SelfLinkedObject *object2 = new SelfLinkedObject(
                QString(), "object2_testObjectRefsChain", 22222222, object1);
    QVERIFY(EnginioTests::createObject(m_client, object2));
    QVERIFY2(!object2->id().isEmpty(), "Object ID not set");

    SelfLinkedObject *object3 = new SelfLinkedObject(
                QString(), "object3_testObjectRefsChain", 33333333, object2);
    QVERIFY(EnginioTests::createObject(m_client, object3));
    QVERIFY2(!object3->id().isEmpty(), "Object ID not set");

    /* Fetch object3 and expand 1 ref level */

    QMap<QString, QString> requestParams;
    requestParams[QStringLiteral("include")] =
            QStringLiteral("{\"objectValue1\":{}}");
    SelfLinkedObject *object3Read = dynamic_cast<SelfLinkedObject*>(
                EnginioTests::readObject(m_client,
                                         object3->id(),
                                         object3->objectType(),
                                         0,
                                         requestParams));

    QVERIFY(object3Read);
    QVERIFY(object3Read->m_objectValue1);
    QCOMPARE(object3Read->m_objectValue1->id(), object2->id());
    QCOMPARE(object3Read->m_objectValue1->m_stringValue, object2->m_stringValue);
    QVERIFY(!object3Read->m_objectValue2);

    delete object3Read;
    object3Read = 0;

    /* Fetch Object3 and expand 2 ref levels */

    requestParams[QStringLiteral("include")] =
            QStringLiteral("{ \"objectValue1\": { \"include\": { \"objectValue1\": {} } } }");
    object3Read = dynamic_cast<SelfLinkedObject*>(
                EnginioTests::readObject(m_client,
                                         object3->id(),
                                         object3->objectType(),
                                         0,
                                         requestParams));
    QVERIFY(object3Read);

    // First ref level
    QVERIFY(object3Read->m_objectValue1);
    QCOMPARE(object3Read->m_objectValue1->id(), object2->id());
    QCOMPARE(object3Read->m_objectValue1->m_stringValue, object2->m_stringValue);
    QVERIFY(!object3Read->m_objectValue2);

    // Second ref level
    QVERIFY(object3Read->m_objectValue1->m_objectValue1);
    QCOMPARE(object3Read->m_objectValue1->m_objectValue1->id(), object1->id());
    QCOMPARE(object3Read->m_objectValue1->m_objectValue1->m_stringValue,
             object1->m_stringValue);
    QVERIFY(!object3Read->m_objectValue1->m_objectValue2);

    delete object3Read;
    delete object1;
    delete object2;
    delete object3;

    m_client->unregisterObjectFactory(factoryId);
}

/*!
 * Create and read object containing reference itself.
 */
void ObjectOperationTest::testSelfLinkingObjectRefs()
{
    QVERIFY2(m_client, "Null client");

    int factoryId = m_client->registerObjectFactory(new TestObjectFactory());
    QVERIFY2(factoryId > 0, "Invalid factory registration ID");

    /* Create Object */
    SelfLinkedObject *object = new SelfLinkedObject(
                QString(), "object_testSelfLinkingObjectRefs");
    QVERIFY(EnginioTests::createObject(m_client, object));
    QVERIFY2(!object->id().isEmpty(), "Object ID not set");

    /* Update Object to have self pointing ref */

    object->m_objectValue1 = object;
    object->m_stringValue.append("_updated");
    EnginioObjectOperation *op = new EnginioObjectOperation(m_client);
    op->update(object);

    QSignalSpy finishedSpy1(op, SIGNAL(finished()));
    QSignalSpy errorSpy1(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 0);

    delete op;

    /* Fetch Object */

    SelfLinkedObject *objectRead = dynamic_cast<SelfLinkedObject*>(
                EnginioTests::readObject(m_client,
                                         object->id(),
                                         object->objectType()));

    QVERIFY(objectRead);
    QCOMPARE(objectRead->id(), object->id());
    QCOMPARE(objectRead->objectType(), object->objectType());
    QCOMPARE(objectRead->m_stringValue, object->m_stringValue);

    QVERIFY(objectRead->m_objectValue1);
    QCOMPARE(objectRead->m_objectValue1->id(), object->id());
    QCOMPARE(objectRead->m_objectValue1->objectType(), object->objectType());
    QVERIFY(objectRead->m_objectValue1->m_stringValue.isEmpty());

    delete objectRead;
    objectRead = 0;

    /* Fetch Object and expand 1 ref level */

    QMap<QString, QString> requestParams;
    requestParams[QStringLiteral("include")] =
            QStringLiteral("{\"objectValue1\":{}}");
    objectRead = dynamic_cast<SelfLinkedObject*>(
                EnginioTests::readObject(m_client,
                                         object->id(),
                                         object->objectType(),
                                         0,
                                         requestParams));

    QVERIFY(objectRead);
    QCOMPARE(objectRead->id(), object->id());
    QCOMPARE(objectRead->objectType(), object->objectType());
    QCOMPARE(objectRead->m_stringValue, object->m_stringValue);

    QVERIFY(objectRead->m_objectValue1);
    QCOMPARE(objectRead->m_objectValue1->id(), object->id());
    QCOMPARE(objectRead->m_objectValue1->objectType(), object->objectType());
    QCOMPARE(objectRead->m_objectValue1->m_stringValue, object->m_stringValue);

    delete objectRead;
    delete object;

    m_client->unregisterObjectFactory(factoryId);
}

/*!
 * Update object in backend by deleting a property
 */
void ObjectOperationTest::testDeleteProperty()
{
    QVERIFY2(m_client, "Null client");
    QVERIFY2(m_model, "Null model");

    /* Create new object in backend and add it to model */

    EnginioJsonObject *originalObject = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    originalObject->insert("stringValue", EnginioTests::randomString(10));
    originalObject->insert("intValue", qrand());
    QVERIFY(EnginioTests::createObject(m_client, originalObject, m_model));
    QVERIFY(!originalObject->id().isEmpty());
    QVERIFY(!originalObject->value("stringValue").isUndefined());
    int modelRowCountBefore = m_model->rowCount();

    /* Delete "stringValue" property from object */

    EnginioJsonObject *updateObject = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    QVERIFY2(updateObject, "Object creation failed");
    updateObject->insert("id", originalObject->id());
    updateObject->insert("stringValue", QJsonValue());
    QVERIFY(updateObject->value("stringValue").isNull());

    EnginioObjectOperation *op = new EnginioObjectOperation(m_client);
    op->setModel(m_model);
    op->update(updateObject);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    QVERIFY2(finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    delete op;

    /* Check that object used in update operation was updated correctly */

    QVERIFY(updateObject->value("stringValue").isUndefined());
    QCOMPARE((int)updateObject->value("intValue").toDouble(),
             (int)originalObject->value("intValue").toDouble());
    QVERIFY2(!updateObject->value("createdAt").isUndefined(), "Create time undefined");
    QVERIFY2(!updateObject->value("updatedAt").isUndefined(), "Update time undefined");
    QVERIFY(updateObject->value("createdAt").toString() !=
            updateObject->value("updatedAt").toString());
    delete updateObject;

    /* Check that object in model was updated correctly */

    QCOMPARE(m_model->rowCount(), modelRowCountBefore);
    QModelIndex index = m_model->indexFromId(originalObject->id());
    QVERIFY2(index.isValid(), "Can't find object from model");
    EnginioJsonObject *modelObject2 = dynamic_cast<EnginioJsonObject*>(
                m_model->getObject(index));
    QVERIFY(modelObject2);
    QVERIFY(modelObject2->value("stringValue").isUndefined());
    QCOMPARE((int)modelObject2->value("intValue").toDouble(),
             (int)originalObject->value("intValue").toDouble());

    /* Get updated object from backend */

    EnginioJsonObject *readObject = dynamic_cast<EnginioJsonObject*>(
                EnginioTests::readObject(m_client,
                                         originalObject->id(),
                                         originalObject->objectType()));
    QVERIFY2(readObject, "Failed to read object");
    QVERIFY(modelObject2->value("stringValue").isUndefined());
    QCOMPARE((int)modelObject2->value("intValue").toDouble(),
             (int)originalObject->value("intValue").toDouble());
    delete readObject;
    delete originalObject;
}

QTEST_MAIN(ObjectOperationTest)

#include "tst_objectoperationtest.moc"
