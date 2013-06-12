#ifndef ENGINIOTESTSCOMMON_H
#define ENGINIOTESTSCOMMON_H

#include <QString>
#include <QMap>

class EnginioAbstractObject;
class EnginioClient;
class EnginioObjectModel;

namespace EnginioTests
{
const QByteArray TESTAPP_ID(qgetenv("ENGINIO_BACKEND_ID"));
const QByteArray TESTAPP_SECRET(qgetenv("ENGINIO_BACKEND_SECRET"));
const QString TESTAPP_URL(qgetenv("ENGINIO_API_URL"));
}

#endif // ENGINIOTESTSCOMMON_H
