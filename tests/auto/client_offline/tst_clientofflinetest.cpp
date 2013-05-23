#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include "enginioclient.h"
#include "enginiojsonobject.h"
#include "enginioobjectmodel.h"
#include "enginioacl.h"

class ClientOfflineTest : public QObject
{
    Q_OBJECT

public:
    ClientOfflineTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCreateClient();
    void testCreateObjectFromClient();
    void testCreateJsonObject();
    void testJsonObjectSerialization();
    void testObjectModel();
    void testAcl();
    void testAclSerialization();
    void testTimeFormat();
};

ClientOfflineTest::ClientOfflineTest()
{
}

void ClientOfflineTest::initTestCase()
{
}

void ClientOfflineTest::cleanupTestCase()
{
}

void ClientOfflineTest::testCreateClient()
{
    const QString BACKEND_ID("xxx-backendId-yyy");
    const QString BACKEND_SECRET("xxx-backendSecret-yyy");

    EnginioClient *client = new EnginioClient(BACKEND_ID, BACKEND_SECRET);
    QVERIFY2(client, "Client creation failed");
    QCOMPARE(client->backendId(), BACKEND_ID);
    QCOMPARE(client->backendSecret(), BACKEND_SECRET);

    QUrl apiUrl = client->apiUrl();
    QVERIFY2(apiUrl.isValid(), "Invalid API URL");

    delete client;
}

void ClientOfflineTest::testCreateObjectFromClient()
{
    const QString OBJECT_TYPE("objects.mytype");
    const QString OBJECT_ID("123-objectid-456");

    EnginioClient *client = new EnginioClient("id", "secret");
    QVERIFY2(client, "Client creation failed");

    EnginioJsonObject *obj = dynamic_cast<EnginioJsonObject*>(
            client->createObject(OBJECT_TYPE, OBJECT_ID));
    QVERIFY2(obj, "Object creation failed");
    QCOMPARE(obj->objectType(), OBJECT_TYPE);
    QCOMPARE(obj->id(), OBJECT_ID);

    delete obj;
    delete client;
}

void ClientOfflineTest::testCreateJsonObject()
{
    const QString OBJECT_TYPE("objects.mytype");
    const QString OBJECT_ID("111-mytype-object-000");
    const bool BOOLEAN_VALUE(false);

    EnginioJsonObject *obj = new EnginioJsonObject(OBJECT_TYPE);
    QVERIFY2(obj, "Object creation failed");
    QCOMPARE(obj->objectType(), OBJECT_TYPE);
    QCOMPARE(obj->id(), QString());

    obj->insert("id", OBJECT_ID);
    QCOMPARE(obj->id(), OBJECT_ID);

    obj->insert("booleanValue", BOOLEAN_VALUE);
    QCOMPARE(obj->value("booleanValue").toBool(), BOOLEAN_VALUE);

    delete obj;
}

void ClientOfflineTest::testJsonObjectSerialization()
{
    const QString OBJECT_TYPE("mytype");
    const QString OBJECT_ID("mytypeID");
    const bool BOOLEAN_VALUE(true);
    const int INT_VALUE(20012001);

    EnginioJsonObject *source = new EnginioJsonObject();
    QVERIFY2(source, "Source object creation failed");
    source->insert("objectType", OBJECT_TYPE);
    source->insert("id", OBJECT_ID);
    source->insert("booleanValue", BOOLEAN_VALUE);
    source->insert("intValue", INT_VALUE);

    QByteArray json = source->toEnginioJson();
    QVERIFY2(!json.isEmpty(), "Empty JSON");

    delete source;

    EnginioJsonObject *dest = new EnginioJsonObject();
    QVERIFY2(source, "Destination object creation failed");
    dest->fromEnginioJson(QJsonDocument::fromJson(json).object());
    QCOMPARE(dest->objectType(), OBJECT_TYPE);
    QCOMPARE(dest->id(), OBJECT_ID);
    QCOMPARE(dest->value("booleanValue").toBool(), BOOLEAN_VALUE);
    QCOMPARE((int)dest->value("intValue").toDouble(), INT_VALUE);

    delete dest;
}

void ClientOfflineTest::testObjectModel()
{
    const QString OBJECT_TYPE("mytype");
    const int N_OBJECTS = 5;

    // Create model

    EnginioObjectModel *model = new EnginioObjectModel();
    QVERIFY2(model, "Model creation failed");
    QCOMPARE(model->rowCount(), 0);

    // Insert objects to model

    QList<EnginioAbstractObject*> objectList;
    for (int i = 0; i < N_OBJECTS; i++) {
        QString id = QString("object-%1").arg(i);
        EnginioJsonObject *o = new EnginioJsonObject(OBJECT_TYPE);
        QVERIFY2(o, "Object creation failed");
        o->insert("id", id);
        o->insert("number", i);
        objectList.append(o);
    }

    QSignalSpy insertSpy(model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy insertRowCountSpy(model, SIGNAL(rowCountChanged(int)));
    QVERIFY2(model->addToModel(objectList, -1), "addToModel failed");
    QCOMPARE(model->rowCount(), N_OBJECTS);
    QCOMPARE(insertSpy.count(), 1);
    QList<QVariant> args = insertSpy.takeFirst();
    QCOMPARE(args.at(1).toInt(), 0); // start index
    QCOMPARE(args.at(2).toInt(), N_OBJECTS - 1); // end index
    QCOMPARE(insertRowCountSpy.count(), 1);
    QCOMPARE(insertRowCountSpy.takeFirst().at(0).toInt(), N_OBJECTS);

    // Get object from model

    QModelIndex index = model->index(0);
    QVERIFY2(index.isValid(), "Invalid index");
    QVariant var = model->data(index, Enginio::DataRole);
    EnginioAbstractObject *obj = var.value<EnginioAbstractObject*>();
    QVERIFY2(obj, "Failed to get row data");
    EnginioJsonObject *jsonObj = dynamic_cast<EnginioJsonObject*>(obj);
    QVERIFY2(jsonObj, "Failed to cast row data");
    QCOMPARE(jsonObj->value("number").toDouble(), 0.0);

    // Update object in model

    jsonObj->insert("number", N_OBJECTS);
    QSignalSpy changeSpy(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
    obj = dynamic_cast<EnginioAbstractObject*>(jsonObj);
    QVERIFY2(model->setData(index, QVariant::fromValue(obj), Enginio::DataRole),
             "setData failed");
    QCOMPARE(changeSpy.count(), 1);

    obj = model->getObject(model->index(0));
    jsonObj = dynamic_cast<EnginioJsonObject*>(obj);
    QCOMPARE((int)jsonObj->value("number").toDouble(), N_OBJECTS);

    // Remove objects from model

    QSignalSpy removeSpy(model, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    QSignalSpy removeRowCountSpy(model, SIGNAL(rowCountChanged(int)));
    // remove all objects except first and last
    QVERIFY2(model->removeFromModel(1, N_OBJECTS - 2), "removeFromModel failed");
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(removeSpy.count(), 1);
    args = removeSpy.takeFirst();
    QCOMPARE(args.at(1).toInt(), 1); // start index
    QCOMPARE(args.at(2).toInt(), N_OBJECTS - 2); // end index
    QCOMPARE(removeRowCountSpy.count(), 1);
    QCOMPARE(removeRowCountSpy.takeFirst().at(0).toInt(), 2);

    delete model;
}

void ClientOfflineTest::testAcl()
{
    QPair<QString, QString> everybody = qMakePair(QStringLiteral("*"),
                                                  QStringLiteral("aclSubject"));
    QPair<QString, QString> user = qMakePair(QStringLiteral("11111111"),
                                             QStringLiteral("users"));
    QPair<QString, QString> group = qMakePair(QStringLiteral("22222222"),
                                              QStringLiteral("usergroups"));
    EnginioAcl *acl = new EnginioAcl();

    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::ReadPermission).size(), 0);
    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::UpdatePermission).size(), 0);
    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::DeletePermission).size(), 0);
    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::AdminPermission).size(), 0);

    acl->addSubjectForPermission(everybody, EnginioAcl::ReadPermission);
    acl->addSubjectForPermission(user, EnginioAcl::DeletePermission);
    acl->addSubjectForPermission(group, EnginioAcl::DeletePermission);

    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::ReadPermission).size(), 1);
    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::UpdatePermission).size(), 0);
    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::DeletePermission).size(), 2);
    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::AdminPermission).size(), 0);

    QVERIFY(acl->isSubjectHavingPermission(everybody, EnginioAcl::ReadPermission));
    QVERIFY(acl->isSubjectHavingPermission(user, EnginioAcl::DeletePermission));
    QVERIFY(acl->isSubjectHavingPermission(group, EnginioAcl::DeletePermission));

    QVERIFY(!acl->isSubjectHavingPermission(everybody, EnginioAcl::AdminPermission));
    QVERIFY(!acl->isSubjectHavingPermission(user, EnginioAcl::AdminPermission));
    QVERIFY(!acl->isSubjectHavingPermission(group, EnginioAcl::AdminPermission));

    acl->addSubjectForPermission(everybody, EnginioAcl::ReadPermission);
    acl->addSubjectForPermission(everybody, EnginioAcl::ReadPermission);

    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::ReadPermission).size(), 1);

    QVERIFY(acl->removeSubjectFromPermission(everybody, EnginioAcl::ReadPermission));
    QVERIFY(acl->removeSubjectFromPermission(user, EnginioAcl::DeletePermission));
    QVERIFY(acl->removeSubjectFromPermission(group, EnginioAcl::DeletePermission));

    QVERIFY(!acl->isSubjectHavingPermission(everybody, EnginioAcl::ReadPermission));
    QVERIFY(!acl->isSubjectHavingPermission(user, EnginioAcl::DeletePermission));
    QVERIFY(!acl->isSubjectHavingPermission(group, EnginioAcl::DeletePermission));

    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::ReadPermission).size(), 0);
    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::UpdatePermission).size(), 0);
    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::DeletePermission).size(), 0);
    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::AdminPermission).size(), 0);

    QVERIFY(!acl->removeSubjectFromPermission(everybody, EnginioAcl::ReadPermission));

    delete acl;
}

void ClientOfflineTest::testAclSerialization()
{
    const QString ACL_JSON =
    "{"
        "\"read\": ["
            "{"
                "\"id\": \"*\","
                "\"objectType\": \"aclSubject\""
            "}"
        "],"
        "\"update\": [],"
        "\"delete\": [],"
        "\"admin\": ["
            "{"
                "\"id\": \"50dac877defbca7067000001\","
                "\"objectType\": \"users\""
            "},"
            "{"
                "\"id\": \"50dac877defbca7067000001\","
                "\"objectType\": \"usergroups\""
            "}"
        "]"
    "}";

    EnginioAcl *sourceAcl = new EnginioAcl();
    sourceAcl->fromJson(ACL_JSON.toUtf8());

    EnginioAcl *acl = new EnginioAcl();
    acl->fromJson(sourceAcl->toJson());

    QPair<QString, QString> everybody = qMakePair(QStringLiteral("*"),
                                                  QStringLiteral("aclSubject"));
    QPair<QString, QString> user = qMakePair(QStringLiteral("50dac877defbca7067000001"),
                                             QStringLiteral("users"));
    QPair<QString, QString> group = qMakePair(QStringLiteral("50dac877defbca7067000001"),
                                              QStringLiteral("usergroups"));

    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::ReadPermission).size(), 1);
    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::UpdatePermission).size(), 0);
    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::DeletePermission).size(), 0);
    QCOMPARE(acl->getSubjectsForPermission(EnginioAcl::AdminPermission).size(), 2);
    QVERIFY(acl->isSubjectHavingPermission(everybody, EnginioAcl::ReadPermission));
    QVERIFY(!acl->isSubjectHavingPermission(everybody, EnginioAcl::UpdatePermission));
    QVERIFY(acl->isSubjectHavingPermission(user, EnginioAcl::AdminPermission));
    QVERIFY(acl->isSubjectHavingPermission(group, EnginioAcl::AdminPermission));

    delete acl;
    delete sourceAcl;
}

/*!
 * Test that EnginioAbstractObject::timeFormat() returns valid time format
 * string.
 */
void ClientOfflineTest::testTimeFormat()
{
    QString timeFormat = EnginioAbstractObject::timeFormat();
    QDateTime time1 = QDateTime::currentDateTime();
    QDateTime time2 = QDateTime::fromString(time1.toString(timeFormat), timeFormat);
    QCOMPARE(time1, time2);
}

QTEST_MAIN(ClientOfflineTest)

#include "tst_clientofflinetest.moc"
