#include "common.h"
#include "enginioabstractobject.h"
#include "enginioidentityauthoperation.h"
#include "enginioobjectmodel.h"
#include "enginioobjectoperation.h"
#include "enginioerror.h"

#include <QSignalSpy>
#include <QDebug>

bool EnginioTests::createObject(EnginioClient *client,
                                EnginioAbstractObject *object,
                                EnginioObjectModel *model)
{
    if (!client || !object)
        return false;

    EnginioObjectOperation *op = new EnginioObjectOperation(client, model);
    if (!op)
        return false;

    op->create(object);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    bool result = finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT);
    result = result && (errorSpy.count() == 0);
    if (!result) {
        EnginioError * error = errorSpy[0][0].value<EnginioError*>();
        qDebug() << error->errorString() << error->error() << error->httpCode();
    }
    delete op;
    return result;
}

EnginioAbstractObject * EnginioTests::readObject(EnginioClient *client,
                                                 const QString &id,
                                                 const QString &objectType,
                                                 EnginioObjectModel *model,
                                                 QMap<QString, QString> requestParams)
{
    if (!client || id.isEmpty() || objectType.isEmpty())
        return 0;

    EnginioObjectOperation *op = new EnginioObjectOperation(client, model);
    if (!op)
        return 0;

    QMap<QString, QString>::ConstIterator iter = requestParams.constBegin();
    while (iter != requestParams.constEnd()) {
        op->setRequestParam(iter.key(), iter.value());
        iter++;
    }

    EnginioAbstractObject *readObject = 0;
    readObject = op->read(id, objectType);
    if (!readObject)
        return 0;

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    bool result = finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT);
    result = result && (errorSpy.count() == 0);
    delete op;
    if (!result) {
        delete readObject;
        return 0;
    }

    return readObject;
}

bool EnginioTests::login(EnginioClient *client,
                         const QString username,
                         const QString password)
{
    if (!client || username.isEmpty() || password.isEmpty())
        return false;

    EnginioIdentityAuthOperation *op = new EnginioIdentityAuthOperation(client);
    if (!op)
        return false;

    op->loginWithUsernameAndPassword(username, password);

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    bool result = finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT);
    result = result && (errorSpy.count() == 0);
    delete op;
    return result;
}

bool EnginioTests::logout(EnginioClient *client)
{
    if (!client)
        return false;

    EnginioIdentityAuthOperation *op = new EnginioIdentityAuthOperation(client);
    if (!op)
        return false;

    op->logout();

    QSignalSpy finishedSpy(op, SIGNAL(finished()));
    QSignalSpy errorSpy(op, SIGNAL(error(EnginioError*)));

    op->execute();

    bool result = finishedSpy.wait(EnginioTests::NETWORK_TIMEOUT);
    result = result && (errorSpy.count() == 0);
    delete op;
    return result;
}

QString EnginioTests::randomString(int length,
                                   const char *allowedChars,
                                   int numAllowedChars)
{
    QString username = QString(length, QChar('x'));
    for (int i = 0; i < username.size(); i++) {
        username[i] = QChar(allowedChars[qrand() % (numAllowedChars - 1)]);
    }
    return username;
}
