#ifndef ENGINIOTESTSCOMMON_H
#define ENGINIOTESTSCOMMON_H

#include <QString>
#include <QMap>

class EnginioAbstractObject;
class EnginioClient;
class EnginioObjectModel;

namespace EnginioTests
{

// Production
const QString TESTAPP_ID("5118f33d5a3d8b0ef9004f31");
const QString TESTAPP_SECRET("e467813ef35abece844df968e61e1824");
const QString TESTAPP_URL("https://api.engin.io");

// Staging
//const QString TESTAPP_ID("5138456cb094254c4a0001f0");
//const QString TESTAPP_SECRET("fced61b838a603b97f034db923970dac");
//const QString TESTAPP_URL("https://api.staging.engin.io");

const QString TEST_OBJECT_TYPE("objects.SelfLinkedObject");
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
