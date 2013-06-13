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
#include <Enginio/private/enginioclient_p.h>
#include <Enginio/enginioreply.h>
#include <Enginio/enginioidentity.h>

#include "../common/common.h"

// For this test to work, there needs to be a property "fileAttachment"
// for "objects.files" that is a ref to files.

class tst_Files: public QObject
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
    void init();
    void fileUploadDownload_data();
    void fileUploadDownload();
};

void tst_Files::init()
{
    if (EnginioTests::TESTAPP_ID.isEmpty() || EnginioTests::TESTAPP_SECRET.isEmpty() || EnginioTests::TESTAPP_URL.isEmpty())
        QFAIL("Needed environment variables ENGINIO_BACKEND_ID, ENGINIO_BACKEND_SECRET, ENGINIO_API_URL are not set!");
}

void tst_Files::fileUploadDownload_data()
{
    QTest::addColumn<int>("chunkSize");

    QTest::newRow("Multi Part") << -1;
    // With such a small chunk size the image will be uploaded in chunks
    QTest::newRow("Chunked") << 1024;
}

void tst_Files::fileUploadDownload()
{
    QFETCH(int, chunkSize);

    EnginioClient client;
    QObject::connect(&client, SIGNAL(error(EnginioReply *)), this, SLOT(error(EnginioReply *)));
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);

    if (chunkSize > 0) {
        EnginioClientPrivate *clientPrivate = EnginioClientPrivate::get(&client);
        clientPrivate->_uploadChunkSize = chunkSize;
    }

    QSignalSpy spy(&client, SIGNAL(finished(EnginioReply *)));
    QSignalSpy spyError(&client, SIGNAL(error(EnginioReply*)));

    //![upload-create-object]
    QJsonObject obj;
    obj["objectType"] = QString::fromUtf8("objects.files");
    obj["title"] = QString::fromUtf8("Object With File");
    const EnginioReply* createReply = client.create(obj);
    //![upload-create-object]
    QVERIFY(createReply);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spyError.count(), 0);

    const EnginioReply *responseObjectCreation = spy[0][0].value<EnginioReply*>();
    QCOMPARE(responseObjectCreation, createReply);
    QCOMPARE(responseObjectCreation->errorCode(), QNetworkReply::NoError);
    QJsonObject data = responseObjectCreation->data();
    QVERIFY(!data.isEmpty());
    QCOMPARE(data["title"], obj["title"]);
    QCOMPARE(data["objectType"], obj["objectType"]);
    QString id = data["id"].toString();
    QVERIFY(!id.isEmpty());



    QString fileName = QStringLiteral("test.png");
    QString path = QStringLiteral(TEST_FILE_PATH);
    QVERIFY(QFile::exists(path));

    // Attach file to the object
    {
    // FIXME: make this work for out of source builds
    // FIXME: consider this url mess

    //![upload]
    QJsonObject object;
    object["id"] = id;
    object["objectType"] = QStringLiteral("objects.files");
    object["propertyName"] = QStringLiteral("fileAttachment");;

    QJsonObject fileObject;
    fileObject[QStringLiteral("fileName")] = fileName;

    QJsonObject uploadJson;
    uploadJson[QStringLiteral("targetFileProperty")] = object;
    uploadJson[QStringLiteral("file")] = fileObject;
    const EnginioReply* responseUpload = client.uploadFile(uploadJson, QUrl(path));
    //![upload]

    QVERIFY(responseUpload);
    QTRY_COMPARE(spy.count(), 2);
    QCOMPARE(spyError.count(), 0);
    }

    // Query including files
    {
    QJsonObject obj2;
    obj2 = QJsonDocument::fromJson(
                "{\"include\": {\"fileAttachment\": {}},"
                 "\"objectType\": \"objects.files\","
                 "\"query\": {\"id\": \"" + id.toUtf8() + "\"}}").object();

    const EnginioReply *reply = client.query(obj2);
    QVERIFY(reply);

    QTRY_COMPARE(spy.count(), 3);
    QCOMPARE(spyError.count(), 0);
    const EnginioReply *responseQuery = spy[2][0].value<EnginioReply*>();
    data = responseQuery->data();
    QVERIFY(data["results"].isArray());
    QVERIFY(data["results"].toArray().first().toObject()["fileAttachment"].isObject());
    QVERIFY(!data["results"].toArray().first().toObject()["fileAttachment"].toObject()["url"].toString().isEmpty());
    QCOMPARE(data["results"].toArray().first().toObject()["fileAttachment"].toObject()["fileName"].toString(), fileName);

    QFile file(path);
    double fileSize = (double) file.size();
    QCOMPARE(data["results"].toArray().first().toObject()["fileAttachment"].toObject()["fileSize"].toDouble(), fileSize);
    }

    // Download
    {
    //![download]
    QJsonObject object;
    object["id"] = id; // ID of an existing object with attached file
    object["objectType"] = QStringLiteral("objects.files");
    object["propertyName"] = QStringLiteral("fileAttachment");;
    const EnginioReply* replyDownload = client.downloadFile(object);
    //![download]

    QVERIFY(replyDownload);
    QTRY_COMPARE(spy.count(), 4);
    QCOMPARE(spyError.count(), 0);
    const EnginioReply *responseDownload = spy[3][0].value<EnginioReply*>();
    QCOMPARE(spy.count(), 4);
    QJsonObject downloadData = responseDownload->data();
    QVERIFY(!downloadData["expiringUrl"].toString().isEmpty());
    QVERIFY(!downloadData["expiresAt"].toString().isEmpty());
    }
}


QTEST_MAIN(tst_Files)
#include "tst_files.moc"
