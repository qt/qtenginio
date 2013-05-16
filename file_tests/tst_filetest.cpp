#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include "../common/common.h"
#include "enginioclient.h"
#include "enginioerror.h"
#include "enginiofileoperation.h"
#include "enginiojsonobject.h"
#include "enginioobjectoperation.h"

class FileTest : public QObject
{
    Q_OBJECT

public:
    FileTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testUploadDownload();
    void testUploadFromFile();
    void testUploadInvalid();
    void testUploadRefless();
    void testUploadMultipart();

private:
    QPointer<EnginioClient> m_client;
    EnginioJsonObject *m_testObject;

    const QString TEST_IMAGE;
    const QString TEST_IMAGE_PATH;
    const QString TEST_IMAGE_CONTENT_TYPE;
};

FileTest::FileTest() :
    TEST_IMAGE("enginio.png"),
    TEST_IMAGE_PATH(":/images/enginio.png"),
    TEST_IMAGE_CONTENT_TYPE("image/png")
{
    qsrand((uint)QTime::currentTime().msec());
    qRegisterMetaType<EnginioError*>(); // required by QSignalSpy
}

void FileTest::initTestCase()
{
    m_client = new EnginioClient(EnginioTests::TESTAPP_ID,
                                 EnginioTests::TESTAPP_SECRET,
                                 this);
    QVERIFY2(m_client, "Client creation failed");
    m_client->setApiUrl(EnginioTests::TESTAPP_URL);

    // create TEST_OBJECT_TYPE object
    m_testObject = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    m_testObject->insert("stringValue", QString("File test object #%1").arg(qrand()));
    QVERIFY(EnginioTests::createObject(m_client, m_testObject));
    QVERIFY(!m_testObject->id().isEmpty());
}

void FileTest::cleanupTestCase()
{
    delete m_testObject;
}

/*!
 * 1. Create new object
 * 2. Open image file, upload it and attach it to object
 * 3. Download object and attached file info
 */
void FileTest::testUploadDownload()
{
    QVERIFY2(m_client, "Null client");

    /* Create new object */

    EnginioJsonObject *object = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    object->insert("stringValue", QString("testUploadDownload #%1").arg(qrand()));
    QVERIFY(EnginioTests::createObject(m_client, object));
    QVERIFY(!object->id().isEmpty());

    /* Upload image file */

    QFile *file = new QFile(TEST_IMAGE_PATH);
    QVERIFY(file->exists());
    file->open(QIODevice::ReadOnly);
    QVERIFY(file->isReadable());

    EnginioFileOperation *fileOp = new EnginioFileOperation(m_client);
    QSignalSpy uploadStatusSpy(fileOp, SIGNAL(uploadStatusChanged()));
    QSignalSpy uploadProgressSpy(fileOp, SIGNAL(uploadProgressChanged()));
    QCOMPARE(fileOp->uploadProgress(), 0.0);

    fileOp->upload(file,
                   TEST_IMAGE,
                   TEST_IMAGE_CONTENT_TYPE,
                   object->objectType(),
                   object->id(),
                   Q_INT64_C(4 * 1024));

    QSignalSpy finishedSpy1(fileOp, SIGNAL(finished()));
    QSignalSpy errorSpy1(fileOp, SIGNAL(error(EnginioError*)));

    fileOp->execute();

    QVERIFY2(finishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation timeout");
    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 0);
    QVERIFY(!fileOp->fileId().isEmpty());
    QCOMPARE(uploadStatusSpy.count(), 3); // unknown -> empty, empty -> incomplete, incomplete -> complete
    QCOMPARE(fileOp->uploadStatus(), EnginioFileOperation::UploadStatusComplete);
    QCOMPARE(uploadProgressSpy.count(), 2);
    QCOMPARE(fileOp->uploadProgress(), 100.0);

    /* Add image reference to object */

    QJsonObject fileRef;
    fileRef.insert("id", fileOp->fileId());
    fileRef.insert("objectType", QStringLiteral("files"));
    object->insert("file", fileRef);

    EnginioObjectOperation *objOp = new EnginioObjectOperation(m_client);
    objOp->update(object);

    QSignalSpy finishedSpy2(objOp, SIGNAL(finished()));
    QSignalSpy errorSpy2(objOp, SIGNAL(error(EnginioError*)));

    objOp->execute();

    QVERIFY2(finishedSpy2.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation timeout");
    QCOMPARE(finishedSpy2.count(), 1);
    QCOMPARE(errorSpy2.count(), 0);

    /* Download object and attached file info */

    QMap<QString, QString> requestParams;
    requestParams[QStringLiteral("include")] = QStringLiteral("{\"file\":{}}");
    EnginioJsonObject *readObject = dynamic_cast<EnginioJsonObject*>(
                EnginioTests::readObject(m_client,
                                         object->id(),
                                         object->objectType(),
                                         0,
                                         requestParams));

    QVERIFY(readObject);
    QJsonObject objectFile = readObject->value("file").toObject();
    QVERIFY(!objectFile.isEmpty());
    QCOMPARE(objectFile.value("id").toString(),
             fileRef.value("id").toString());
    QCOMPARE(objectFile.value("objectType").toString(),
             fileRef.value("objectType").toString());
    QCOMPARE(objectFile.value("fileName").toString(), TEST_IMAGE);
    QCOMPARE(objectFile.value("contentType").toString(), TEST_IMAGE_CONTENT_TYPE);
    QCOMPARE((qint64)objectFile.value("fileSize").toDouble(), file->size());
    QVERIFY(!objectFile.value("url").toString().isEmpty());

    file->close();
    delete fileOp;
    delete object;
}

/*!
 * 1. Create new object
 * 2. Upload image file and attach it to object
 * 3. Download object and attached file info
 */
void FileTest::testUploadFromFile()
{
    QVERIFY2(m_client, "Null client");

    /* Create new object */

    EnginioJsonObject *object = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    object->insert("stringValue", QString("testUploadFromFile #%1").arg(qrand()));
    QVERIFY(EnginioTests::createObject(m_client, object));
    QVERIFY(!object->id().isEmpty());

    /* Upload image file */

    EnginioFileOperation *fileOp = new EnginioFileOperation(m_client);
    QSignalSpy uploadStatusSpy(fileOp, SIGNAL(uploadStatusChanged()));

    fileOp->upload(TEST_IMAGE_PATH,
                   TEST_IMAGE_CONTENT_TYPE,
                   object->objectType(),
                   object->id(),
                   Q_INT64_C(4 * 1024));

    QSignalSpy finishedSpy1(fileOp, SIGNAL(finished()));
    QSignalSpy errorSpy1(fileOp, SIGNAL(error(EnginioError*)));

    fileOp->execute();

    QVERIFY2(finishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation timeout");
    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 0);
    QVERIFY(!fileOp->fileId().isEmpty());
    QCOMPARE(uploadStatusSpy.count(), 3); // unknown -> empty, empty -> incomplete, incomplete -> complete
    QCOMPARE(fileOp->uploadStatus(), EnginioFileOperation::UploadStatusComplete);

    /* Add image reference to object */

    QJsonObject fileRef;
    fileRef.insert("id", fileOp->fileId());
    fileRef.insert("objectType", QStringLiteral("files"));
    object->insert("file", fileRef);

    EnginioObjectOperation *objOp = new EnginioObjectOperation(m_client);
    objOp->update(object);

    QSignalSpy finishedSpy2(objOp, SIGNAL(finished()));
    QSignalSpy errorSpy2(objOp, SIGNAL(error(EnginioError*)));

    objOp->execute();

    QVERIFY2(finishedSpy2.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation timeout");
    QCOMPARE(finishedSpy2.count(), 1);
    QCOMPARE(errorSpy2.count(), 0);

    /* Download object and attached file info */

    QMap<QString, QString> requestParams;
    requestParams[QStringLiteral("include")] = QStringLiteral("{\"file\":{}}");
    EnginioJsonObject *readObject = dynamic_cast<EnginioJsonObject*>(
                EnginioTests::readObject(m_client,
                                         object->id(),
                                         object->objectType(),
                                         0,
                                         requestParams));

    QVERIFY(readObject);
    QJsonObject objectFile = readObject->value("file").toObject();
    QVERIFY(!objectFile.isEmpty());
    QCOMPARE(objectFile.value("id").toString(),
             fileRef.value("id").toString());
    QCOMPARE(objectFile.value("objectType").toString(),
             fileRef.value("objectType").toString());
    QCOMPARE(objectFile.value("fileName").toString(), TEST_IMAGE);
    QCOMPARE(objectFile.value("contentType").toString(), TEST_IMAGE_CONTENT_TYPE);
    QVERIFY(!objectFile.value("url").toString().isEmpty());

    delete fileOp;
    delete object;
}

/*!
 * Try to upload file from invalid path.
 */
void FileTest::testUploadInvalid()
{
    QVERIFY2(m_client, "Null client");

    EnginioFileOperation *fileOp = new EnginioFileOperation(m_client);
    QSignalSpy uploadStatusSpy(fileOp, SIGNAL(uploadStatusChanged()));

    fileOp->upload(QStringLiteral("dummy/path"),
                   TEST_IMAGE_CONTENT_TYPE,
                   EnginioTests::TEST_OBJECT_TYPE,
                   "fakeId");

    QSignalSpy finishedSpy1(fileOp, SIGNAL(finished()));
    QSignalSpy errorSpy1(fileOp, SIGNAL(error(EnginioError*)));

    fileOp->execute();

    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 1);
    QVERIFY(fileOp->fileId().isEmpty());
    QCOMPARE(uploadStatusSpy.count(), 0);
    QCOMPARE(fileOp->uploadStatus(), EnginioFileOperation::UploadStatusUnknown);

    delete fileOp;
}

/*!
 * 1. Upload image file
 * 2. Create new object with reference to file
 * 3. Download object and attached file info
 */
void FileTest::testUploadRefless()
{
    QVERIFY2(m_client, "Null client");

    /* Upload image file */

    EnginioFileOperation *fileOp = new EnginioFileOperation(m_client);
    QSignalSpy uploadStatusSpy(fileOp, SIGNAL(uploadStatusChanged()));

    fileOp->upload(TEST_IMAGE_PATH,
                   TEST_IMAGE_CONTENT_TYPE,
                   EnginioTests::TEST_OBJECT_TYPE,
                   QString(),
                   Q_INT64_C(4 * 1024));

    QSignalSpy finishedSpy1(fileOp, SIGNAL(finished()));
    QSignalSpy errorSpy1(fileOp, SIGNAL(error(EnginioError*)));

    fileOp->execute();

    QVERIFY2(finishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation timeout");
    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 0);
    QVERIFY(!fileOp->fileId().isEmpty());
    QCOMPARE(uploadStatusSpy.count(), 3); // unknown -> empty, empty -> incomplete, incomplete -> complete
    QCOMPARE(fileOp->uploadStatus(), EnginioFileOperation::UploadStatusComplete);

    /* Create new object containing reference to image */

    EnginioJsonObject *object = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    object->insert("stringValue", QString("testUploadRefless #%1").arg(qrand()));

    QJsonObject fileRef;
    fileRef.insert("id", fileOp->fileId());
    fileRef.insert("objectType", QStringLiteral("files"));
    object->insert("file", fileRef);

    QVERIFY(EnginioTests::createObject(m_client, object));
    QVERIFY(!object->id().isEmpty());

    /* Download object and attached file info */

    QMap<QString, QString> requestParams;
    requestParams[QStringLiteral("include")] = QStringLiteral("{\"file\":{}}");
    EnginioJsonObject *readObject = dynamic_cast<EnginioJsonObject*>(
                EnginioTests::readObject(m_client,
                                         object->id(),
                                         object->objectType(),
                                         0,
                                         requestParams));

    QVERIFY(readObject);
    QCOMPARE(readObject->id(), object->id());
    QJsonObject objectFile = readObject->value("file").toObject();
    QVERIFY(!objectFile.isEmpty());
    QCOMPARE(objectFile.value("id").toString(),
             fileRef.value("id").toString());
    QCOMPARE(objectFile.value("objectType").toString(),
             fileRef.value("objectType").toString());
    QCOMPARE(objectFile.value("fileName").toString(), TEST_IMAGE);
    QCOMPARE(objectFile.value("contentType").toString(), TEST_IMAGE_CONTENT_TYPE);
    QVERIFY(!objectFile.value("url").toString().isEmpty());

    delete fileOp;
    delete object;
}

/*!
 * 1. Upload image file using default chunk size. Chunk size should be bigger
 *    than size of the test image so that image is uploaded as multipart
 *    request.
 * 2. Create new object with reference to file
 * 3. Download object and attached file info
 */

void FileTest::testUploadMultipart()
{
    QVERIFY2(m_client, "Null client");

    /* Upload image file */

    EnginioFileOperation *fileOp = new EnginioFileOperation(m_client);

    QSignalSpy uploadStatusSpy(fileOp, SIGNAL(uploadStatusChanged()));
    QSignalSpy uploadProgressSpy(fileOp, SIGNAL(uploadProgressChanged()));
    QCOMPARE(fileOp->uploadProgress(), 0.0);

    fileOp->upload(TEST_IMAGE_PATH,
                   TEST_IMAGE_CONTENT_TYPE,
                   EnginioTests::TEST_OBJECT_TYPE);

    QSignalSpy finishedSpy1(fileOp, SIGNAL(finished()));
    QSignalSpy errorSpy1(fileOp, SIGNAL(error(EnginioError*)));

    fileOp->execute();

    QVERIFY2(finishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation timeout");
    QCOMPARE(finishedSpy1.count(), 1);
    QCOMPARE(errorSpy1.count(), 0);
    QVERIFY(!fileOp->fileId().isEmpty());
    QCOMPARE(uploadStatusSpy.count(), 1); // unknown -> complete
    QCOMPARE(fileOp->uploadStatus(), EnginioFileOperation::UploadStatusComplete);
    QCOMPARE(uploadProgressSpy.count(), 1);
    QCOMPARE(fileOp->uploadProgress(), 100.0);

    /* Create new object containing reference to image */

    EnginioJsonObject *object = new EnginioJsonObject(EnginioTests::TEST_OBJECT_TYPE);
    object->insert("stringValue", QString("testUploadChunked #%1").arg(qrand()));

    QJsonObject fileRef;
    fileRef.insert("id", fileOp->fileId());
    fileRef.insert("objectType", QStringLiteral("files"));
    object->insert("file", fileRef);

    QVERIFY(EnginioTests::createObject(m_client, object));
    QVERIFY(!object->id().isEmpty());

    /* Download object and attached file info */

    QMap<QString, QString> requestParams;
    requestParams[QStringLiteral("include")] = QStringLiteral("{\"file\":{}}");
    EnginioJsonObject *readObject = dynamic_cast<EnginioJsonObject*>(
                EnginioTests::readObject(m_client,
                                         object->id(),
                                         object->objectType(),
                                         0,
                                         requestParams));

    QVERIFY(readObject);
    QCOMPARE(readObject->id(), object->id());
    QJsonObject objectFile = readObject->value("file").toObject();
    QVERIFY(!objectFile.isEmpty());
    QCOMPARE(objectFile.value("id").toString(),
             fileRef.value("id").toString());
    QCOMPARE(objectFile.value("objectType").toString(),
             fileRef.value("objectType").toString());
    QCOMPARE(objectFile.value("fileName").toString(), TEST_IMAGE);
    QCOMPARE(objectFile.value("contentType").toString(), TEST_IMAGE_CONTENT_TYPE);
    QVERIFY(!objectFile.value("url").toString().isEmpty());

    delete fileOp;
    delete object;
}

QTEST_MAIN(FileTest)

#include "tst_filetest.moc"
