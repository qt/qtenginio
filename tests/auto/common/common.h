#ifndef ENGINIOTESTSCOMMON_H
#define ENGINIOTESTSCOMMON_H

#include <Enginio/enginioclient.h>
#include <QtCore/qurl.h>

class EnginioReply;

namespace EnginioTests
{
const QByteArray TESTAPP_ID(qgetenv("ENGINIO_BACKEND_ID"));
const QByteArray TESTAPP_SECRET(qgetenv("ENGINIO_BACKEND_SECRET"));
const QString TESTAPP_URL(qgetenv("ENGINIO_API_URL"));

class EnginioBackendManager: public QObject
{
    Q_OBJECT

    EnginioClient _client;
    QJsonObject _responseData;
    QString _email;
    QString _password;
    QUrl _url;

    bool synchronousRequest(const QByteArray &httpOperation, const QJsonObject &data = QJsonObject());
    QString authenticate();

public slots:
    void error(EnginioReply *reply);
    void finished(EnginioReply* reply);

public:
    explicit EnginioBackendManager(QObject *parent = 0);
    virtual ~EnginioBackendManager();
    bool createBackend(const QString& backendName);
    bool removeBackend(const QString& backendName);
};

}

#endif // ENGINIOTESTSCOMMON_H
