#ifndef ENGINIOTESTSCOMMON_H
#define ENGINIOTESTSCOMMON_H

#include <Enginio/enginioclient.h>
#include <QtCore/qmap.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qurl.h>

class EnginioReply;

namespace EnginioTests
{

const QByteArray TESTAPP_ID(qgetenv("ENGINIO_BACKEND_ID"));
const QByteArray TESTAPP_SECRET(qgetenv("ENGINIO_BACKEND_SECRET"));
const QString TESTAPP_URL(qgetenv("ENGINIO_API_URL"));
const QString TESTAPP_ENV = QStringLiteral("development");
const QString CUSTOM_OBJECT1(QStringLiteral("CustomObject1"));
const QString CUSTOM_OBJECT2(QStringLiteral("CustomObject2"));

class EnginioBackendManager: public QObject
{
    Q_OBJECT

    EnginioClient _client;
    QJsonObject _headers;
    QJsonObject _responseData;
    QMap<QString, QJsonArray> _backendEnvironments;
    QString _email;
    QString _password;
    QUrl _url;

    bool synchronousRequest(const QByteArray &httpOperation, const QJsonObject &data = QJsonObject());
    bool removeAppWithId(const QString &appId);
    bool authenticate();
    QString getAppId(const QString &backendName);
    QJsonArray getAllBackends();
    QJsonArray getEnvironments(const QString &backendName);

public slots:
    void error(EnginioReply *reply);
    void finished(EnginioReply* reply);

public:
    explicit EnginioBackendManager(QObject *parent = 0);
    virtual ~EnginioBackendManager();
    bool createBackend(const QString &backendName);
    bool removeBackend(const QString &backendName);
    bool createObjectType(const QString &backendName, const QString &environment, const QJsonObject &schema);
    QJsonObject backendApiKeys(const QString &backendName, const QString &environment);
};

void prepareTestUsersAndUserGroups(const QByteArray &backendId, const QByteArray &backendSecret);
}

#endif // ENGINIOTESTSCOMMON_H
