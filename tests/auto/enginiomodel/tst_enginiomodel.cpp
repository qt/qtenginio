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

private slots:
    void init();
    void ctor();
    void enginio_property();
    void query_property();
    void operation_property();
    void roleNames();
    void listView();
};

void tst_EnginioModel::init()
{
    if (EnginioTests::TESTAPP_ID.isEmpty() || EnginioTests::TESTAPP_SECRET.isEmpty() || EnginioTests::TESTAPP_URL.isEmpty())
        QFAIL("Needed environment variables ENGINIO_BACKEND_ID, ENGINIO_BACKEND_SECRET, ENGINIO_API_URL are not set!");
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
        QSignalSpy spyInitilized(&client, SIGNAL(clientInitialized()));
        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
        model.setEnginio(&client);
        QTRY_COMPARE(spyInitilized.count(), 1);
    }
    {   // check if destructor of a fully initilized EnginioClient detach fully initilized model
        EnginioClient client;
        QSignalSpy spyInitilized(&client, SIGNAL(clientInitialized()));
        EnginioModel model;
        model.setOperation(EnginioClient::ObjectOperation);
        model.setQuery(query);
        client.setBackendId(EnginioTests::TESTAPP_ID);
        client.setApiUrl(EnginioTests::TESTAPP_URL);
        client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
        model.setEnginio(&client);
        QTRY_COMPARE(spyInitilized.count(), 1);
    }
}

void tst_EnginioModel::enginio_property()
{
    {
        EnginioClient client;
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
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);
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

    QCOMPARE(roles[Qt::DisplayRole], QByteArrayLiteral("id"));
}

void tst_EnginioModel::listView()
{
    QJsonObject query = QJsonDocument::fromJson("{\"limit\":2}").object();
    QVERIFY(!query.isEmpty());
    EnginioModel model;
    model.setQuery(query);
    model.setOperation(EnginioClient::UserOperation);

    EnginioClient client;
    client.setBackendId(EnginioTests::TESTAPP_ID);
    client.setBackendSecret(EnginioTests::TESTAPP_SECRET);
    client.setApiUrl(EnginioTests::TESTAPP_URL);
    model.setEnginio(&client);

    QListView view;
    view.setModel(&model);
    view.show();

    QTRY_COMPARE(model.rowCount(), 2);
    QVERIFY(view.model() == &model);
    QVERIFY(view.indexAt(QPoint(1,1)).data().isValid());
}

QTEST_MAIN(tst_EnginioModel)
#include "tst_enginiomodel.moc"
