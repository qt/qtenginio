#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include "../common/common.h"
#include "enginioclient.h"
#include "enginioerror.h"
#include "enginiojsonobject.h"
#include "enginioobjectmodel.h"
#include "enginioqueryoperation.h"
#include "enginiousergroupoperation.h"

class UsergroupTests : public QObject
{
    Q_OBJECT
    
public:
    UsergroupTests();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void addRemoveMembers();
    void addRemoveFromModel();

private:
    EnginioClient *m_client;
    QList<EnginioJsonObject*> m_users;
};

UsergroupTests::UsergroupTests()
{
    qsrand((uint)QTime::currentTime().msec());
    qRegisterMetaType<EnginioError*>(); // required by QSignalSpy
}

void UsergroupTests::initTestCase()
{
    m_client = new EnginioClient(EnginioTests::TESTAPP_ID,
                                 EnginioTests::TESTAPP_SECRET,
                                 this);
    QVERIFY2(m_client, "Client creation failed");
    m_client->setApiUrl(EnginioTests::TESTAPP_URL);

    /* Create three new users and add them to m_users list */

    for (int i = 0; i < 3; i++) {
        EnginioJsonObject *user = new EnginioJsonObject("users");
        user->insert("username", EnginioTests::randomString(15));
        user->insert("firstName", QStringLiteral("firstName_UsergroupTests%1").arg(i));
        user->insert("lastName", QStringLiteral("lastName_UsergroupTests%1").arg(i));
        user->insert("password", QStringLiteral("pa$$word"));
        QVERIFY(EnginioTests::createObject(m_client, user));
        QVERIFY(!user->id().isEmpty());
        m_users.append(user);
    }
}

void UsergroupTests::cleanupTestCase()
{
}

void UsergroupTests::addRemoveMembers()
{
    QVERIFY2(m_client, "Null client");

    /* Create new usergroup */

    EnginioJsonObject *group = new EnginioJsonObject("usergroups");
    group->insert("name", EnginioTests::randomString(10));
    QVERIFY(EnginioTests::createObject(m_client, group));
    QVERIFY(!group->id().isEmpty());

    /* Add users to usergroup using different API methods */

    EnginioUsergroupOperation *groupOp;
    QSignalSpy *finishedSpy;
    QSignalSpy *errorSpy;
    for (int i = 0; i < 3; i++) {
        groupOp = new EnginioUsergroupOperation(m_client);
        switch (i) {
        case 0:
            // User object pointer
            groupOp->addMember(m_users[0], group->id());
            break;
        case 1:
            // User object reference
            groupOp->addMember(*m_users[1], group->id());
            break;
        case 2:
            // User ID
            groupOp->addMember(m_users[2]->id(), group->id());
            break;
        default:
            QFAIL("Loop fail");
        }

        finishedSpy = new QSignalSpy(groupOp, SIGNAL(finished()));
        errorSpy = new QSignalSpy(groupOp, SIGNAL(error(EnginioError*)));

        groupOp->execute();

        QVERIFY2(finishedSpy->wait(EnginioTests::NETWORK_TIMEOUT),
                 "Operation was too slow");
        QCOMPARE(finishedSpy->count(), 1);
        QCOMPARE(errorSpy->count(), 0);

        delete groupOp;
        delete finishedSpy;
        delete errorSpy;
    }

    /* Get usergroup members */

    EnginioQueryOperation *queryOp = new EnginioQueryOperation(m_client);
    queryOp->setUsergroupId(group->id());
    QSignalSpy queryFinishedSpy1(queryOp, SIGNAL(finished()));
    QSignalSpy queryErrorSpy1(queryOp, SIGNAL(error(EnginioError*)));

    queryOp->execute();

    QVERIFY2(queryFinishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(queryFinishedSpy1.count(), 1);
    QCOMPARE(queryErrorSpy1.count(), 0);

    QList<EnginioAbstractObject*> results = queryOp->takeResults();
    delete queryOp;

    QCOMPARE(results.size(), 3);
    foreach (EnginioAbstractObject *obj, results) {
        EnginioJsonObject *member = dynamic_cast<EnginioJsonObject*>(obj);
        QVERIFY(member);
        bool matchFound = false;
        foreach (EnginioJsonObject *user, m_users) {
            if (member->id() == user->id()) {
                QCOMPARE(member->objectType(), user->objectType());
                QCOMPARE(member->value("username").toString(),
                         user->value("username").toString());
                QCOMPARE(member->value("firstName").toString(),
                         user->value("firstName").toString());
                QCOMPARE(member->value("lastName").toString(),
                         user->value("lastName").toString());
                QVERIFY(member->value("password").isUndefined());
                matchFound = true;
                break;
            }
        }
        QVERIFY(matchFound);
    }

    while (!results.isEmpty())
        delete results.takeFirst();

    /* Remove users from usergroup using different API methods */

    for (int i = 0; i < 3; i++) {
        groupOp = new EnginioUsergroupOperation(m_client);
        switch (i) {
        case 0:
            // User object pointer
            groupOp->removeMember(m_users[0], group->id());
            break;
        case 1:
            // User object reference
            groupOp->removeMember(*m_users[1], group->id());
            break;
        case 2:
            // User ID
            groupOp->removeMember(m_users[2]->id(), group->id());
            break;
        default:
            QFAIL("Loop fail");
        }

        finishedSpy = new QSignalSpy(groupOp, SIGNAL(finished()));
        errorSpy = new QSignalSpy(groupOp, SIGNAL(error(EnginioError*)));

        groupOp->execute();

        QVERIFY2(finishedSpy->wait(EnginioTests::NETWORK_TIMEOUT),
                 "Operation was too slow");
        QCOMPARE(finishedSpy->count(), 1);
        QCOMPARE(errorSpy->count(), 0);

        delete groupOp;
        delete finishedSpy;
        delete errorSpy;
    }

    /* Get usergroup members */

    queryOp = new EnginioQueryOperation(m_client);
    queryOp->setUsergroupId(group->id());
    QSignalSpy queryFinishedSpy2(queryOp, SIGNAL(finished()));
    QSignalSpy queryErrorSpy2(queryOp, SIGNAL(error(EnginioError*)));

    queryOp->execute();

    QVERIFY2(queryFinishedSpy2.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(queryFinishedSpy2.count(), 1);
    QCOMPARE(queryErrorSpy2.count(), 0);

    results = queryOp->takeResults();
    QCOMPARE(results.size(), 0);
    delete queryOp;
}

void UsergroupTests::addRemoveFromModel()
{
    QVERIFY2(m_client, "Null client");

    /* Create new usergroup */

    EnginioJsonObject *group = new EnginioJsonObject("usergroups");
    group->insert("name", EnginioTests::randomString(10));
    QVERIFY(EnginioTests::createObject(m_client, group));
    QVERIFY(!group->id().isEmpty());

    /* Add users to usergroup (and model) using different API methods */

    EnginioObjectModel *addModel = new EnginioObjectModel();
    EnginioUsergroupOperation *groupOp;
    EnginioJsonObject *modelUser;
    QSignalSpy *finishedSpy;
    QSignalSpy *errorSpy;
    for (int i = 0; i < 3; i++) {
        groupOp = new EnginioUsergroupOperation(m_client);
        groupOp->setModel(addModel);
        switch (i) {
        case 0:
            // User object pointer
            groupOp->addMember(m_users[0], group->id());
            break;
        case 1:
            // User object reference
            groupOp->addMember(*m_users[1], group->id());
            break;
        case 2:
            // User ID
            groupOp->addMember(m_users[2]->id(), group->id());
            break;
        default:
            QFAIL("Loop fail");
        }

        finishedSpy = new QSignalSpy(groupOp, SIGNAL(finished()));
        errorSpy = new QSignalSpy(groupOp, SIGNAL(error(EnginioError*)));

        groupOp->execute();

        QVERIFY2(finishedSpy->wait(EnginioTests::NETWORK_TIMEOUT),
                 "Operation was too slow");
        QCOMPARE(finishedSpy->count(), 1);
        QCOMPARE(errorSpy->count(), 0);

        QCOMPARE(addModel->rowCount(), i+1);
        modelUser = dynamic_cast<EnginioJsonObject*>(addModel->getObject(addModel->index(i)));
        QVERIFY(modelUser);
        QCOMPARE(modelUser->id(), m_users[i]->id());
        QCOMPARE(modelUser->objectType(), QStringLiteral("users"));
        QCOMPARE(modelUser->value("username").toString(),
                 m_users[i]->value("username").toString());
        QCOMPARE(modelUser->value("firstName").toString(),
                 m_users[i]->value("firstName").toString());
        QCOMPARE(modelUser->value("lastName").toString(),
                 m_users[i]->value("lastName").toString());
        QVERIFY(modelUser->value("password").isUndefined());

        delete groupOp;
        delete finishedSpy;
        delete errorSpy;
    }

    delete addModel;

    /* Fetch usergroup members to new model */

    EnginioObjectModel *memberModel = new EnginioObjectModel();
    EnginioQueryOperation *queryOp = new EnginioQueryOperation(m_client);
    queryOp->setUsergroupId(group->id());
    queryOp->setModel(memberModel);
    QSignalSpy queryFinishedSpy1(queryOp, SIGNAL(finished()));
    QSignalSpy queryErrorSpy1(queryOp, SIGNAL(error(EnginioError*)));

    queryOp->execute();

    QVERIFY2(queryFinishedSpy1.wait(EnginioTests::NETWORK_TIMEOUT),
             "Operation was too slow");
    QCOMPARE(queryFinishedSpy1.count(), 1);
    QCOMPARE(queryErrorSpy1.count(), 0);

    delete queryOp;

    QCOMPARE(memberModel->rowCount(), 3);

    foreach (EnginioJsonObject *user, m_users) {
        QModelIndex userIndex = memberModel->indexFromId(user->id());
        QVERIFY(userIndex.isValid());
        EnginioJsonObject *member = dynamic_cast<EnginioJsonObject*>(
                    memberModel->getObject(userIndex));
        QCOMPARE(member->objectType(), user->objectType());
        QCOMPARE(member->value("username").toString(),
                 user->value("username").toString());
        QCOMPARE(member->value("firstName").toString(),
                 user->value("firstName").toString());
        QCOMPARE(member->value("lastName").toString(),
                 user->value("lastName").toString());
        QVERIFY(member->value("password").isUndefined());
    }

    /* Remove users from usergroup (and model) using different API methods */

    for (int i = 2; i >= 0; i--) {
        groupOp = new EnginioUsergroupOperation(m_client);
        groupOp->setModel(memberModel);
        switch (i) {
        case 2:
            // User object pointer
            groupOp->removeMember(m_users[i], group->id());
            break;
        case 1:
            // User object reference
            groupOp->removeMember(*m_users[i], group->id());
            break;
        case 0:
            // User ID
            groupOp->removeMember(m_users[i]->id(), group->id());
            break;
        default:
            QFAIL("Loop fail");
        }

        finishedSpy = new QSignalSpy(groupOp, SIGNAL(finished()));
        errorSpy = new QSignalSpy(groupOp, SIGNAL(error(EnginioError*)));

        groupOp->execute();

        QVERIFY2(finishedSpy->wait(EnginioTests::NETWORK_TIMEOUT),
                 "Operation was too slow");
        QCOMPARE(finishedSpy->count(), 1);
        QCOMPARE(errorSpy->count(), 0);

        QCOMPARE(memberModel->rowCount(), i);
        QVERIFY2(!memberModel->indexFromId(m_users[i]->id()).isValid(),
                 "User found from model after it was removed");

        delete groupOp;
        delete finishedSpy;
        delete errorSpy;
    }

    delete memberModel;
}

QTEST_MAIN(UsergroupTests)

#include "tst_usergrouptest.moc"
