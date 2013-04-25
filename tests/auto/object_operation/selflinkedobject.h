#ifndef SELFLINKEDOBJECT_H
#define SELFLINKEDOBJECT_H

#include "enginioabstractobject.h"

class SelfLinkedObject : public EnginioAbstractObject
{
public:
    // Note: Class must have default constructor
    SelfLinkedObject(const QString &id = QString(),
                     const QString &stringValue = QString(),
                     int intValue = 0,
                     SelfLinkedObject *objectValue1 = 0,
                     SelfLinkedObject *objectValue2 = 0);
    ~SelfLinkedObject();
    QByteArray toEnginioJson(bool isObjectRef = false) const;
    bool fromEnginioJson(const QJsonObject &json);
    QString id() const;
    QString objectType() const { return QString("objects.SelfLinkedObject"); }

public:
    QString m_id;
    QString m_stringValue;
    int m_intValue;
    SelfLinkedObject *m_objectValue1;
    SelfLinkedObject *m_objectValue2;
    bool m_deleteObjectValue1;
    bool m_deleteObjectValue2;
};

#endif // SELFLINKEDOBJECT_H
