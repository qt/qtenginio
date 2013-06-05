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

}

#endif // ENGINIOTESTSCOMMON_H
