#ifndef ENGINIOTESTSCOMMON_H
#define ENGINIOTESTSCOMMON_H

#include <QString>
#include <QMap>

class EnginioAbstractObject;
class EnginioClient;
class EnginioObjectModel;

namespace EnginioTests
{

const QString TESTAPP_ID(qgetenv("ENGINIO_BACKEND_ID"));
const QString TESTAPP_SECRET(qgetenv("ENGINIO_BACKEND_SECRET"));
const QString TESTAPP_URL(qgetenv("ENGINIO_API_URL"));

const QString TEST_OBJECT_TYPE("objects.SelfLinkedObject");
const QString CUSTOM_OBJECT_TYPE("objects.CustomObject");
const int NETWORK_TIMEOUT = 5000;

static const char ALPHANUM_CHARS[] =
        "01234567890"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

/*
 * Creates new object in backend. Returns false on failure.
 */
bool createObject(EnginioClient *client,
                  EnginioAbstractObject *object,
                  EnginioObjectModel *model = 0);

/*
 * Reads and returns object from backend. Returns null pointer on failure.
 */
EnginioAbstractObject * readObject(EnginioClient *client,
                                   const QString &id,
                                   const QString &objectType,
                                   EnginioObjectModel *model = 0,
                                   QMap<QString, QString> requestParams = QMap<QString, QString>());

/*
 * Logs in to engin.io. Returns false on failure.
 */
bool login(EnginioClient *client,
           const QString username,
           const QString password);

/*
 * Logs out authorized user. Returns false on failure.
 */
bool logout(EnginioClient *client);

/*
 * Creates 'length' characters long random string using 'allowedChars'.
 */
QString randomString(int length,
                     const char* allowedChars = ALPHANUM_CHARS,
                     int numAllowedChars = sizeof(ALPHANUM_CHARS));

}

#endif // ENGINIOTESTSCOMMON_H
