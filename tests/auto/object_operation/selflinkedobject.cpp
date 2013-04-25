#include "selflinkedobject.h"
#include <QJsonObject>
#include <QDebug>

/*!
 * \class SelfLinkedObject
 * \brief Class representing custom engin.io object
 */

SelfLinkedObject::SelfLinkedObject(const QString &id,
                                   const QString &stringValue,
                                   int intValue,
                                   SelfLinkedObject *objectValue1,
                                   SelfLinkedObject *objectValue2) :
    m_id(id),
    m_stringValue(stringValue),
    m_intValue(intValue),
    m_objectValue1(objectValue1),
    m_objectValue2(objectValue2),
    m_deleteObjectValue1(false),
    m_deleteObjectValue2(false)
{
}

SelfLinkedObject::~SelfLinkedObject()
{
    /*
     * Delete referenced objects created from JSON. Please note that this works
     * with self references (o1.m_objectValue = o1) but not with bigger
     * reference cycles (o1.m_objectValue = o2; o2.m_objectValue = o1)
     */
    if (m_deleteObjectValue1 && m_objectValue1 && m_objectValue1 != this)
        delete m_objectValue1;
    if (m_deleteObjectValue2 && m_objectValue2 && m_objectValue1 != this)
        delete m_objectValue2;
}

QByteArray SelfLinkedObject::toEnginioJson(bool isObjectRef) const
{
    QByteArray json;

    json += '{';

    json += "\"objectType\":\"";
    json += objectType();
    json += "\"";

    if (!m_id.isEmpty()) {
        json += ",\"id\":\"";
        json += m_id;
        json += "\"";
    }

    if (!isObjectRef) {
        json += ",\"stringValue\":\"";
        json += m_stringValue;
        json += "\"";

        json += ",\"intValue\":";
        json += QByteArray::number(m_intValue);

        if (m_objectValue1) {
            json += ",\"objectValue1\":";
            json += m_objectValue1->toEnginioJson(true);
        }

        if (m_objectValue2) {
            json += ",\"objectValue2\":";
            json += m_objectValue2->toEnginioJson(true);
        }
    }

    json += '}';

    return json;
}

bool SelfLinkedObject::fromEnginioJson(const QJsonObject &json)
{
    QJsonValue val = json["objectType"];
    Q_ASSERT(val.isString());
    Q_ASSERT(val.toString() == objectType());

    val = json["id"];
    if (val.isString())
        m_id = val.toString();
    else
        return false;

    if (json.contains("stringValue")) {
        if (json.value("stringValue").isString())
            m_stringValue = json.value("stringValue").toString();
        else
            return false;
    }

    if (json.contains("intValue")) {
        if (json.value("intValue").isDouble())
            m_intValue = (int)json.value("intValue").toDouble();
        else
            return false;
    }

    if (json.contains("objectValue1")) {
        bool valid = true;
        if (json.value("objectValue1").isObject()) {
            if (!m_objectValue1) {
                m_objectValue1 = new SelfLinkedObject();
                m_deleteObjectValue1 = true;
            }
            valid = valid && m_objectValue1->fromEnginioJson(
                        json.value("objectValue1").toObject());
        } else
            valid = false;

        if (!valid)
            return false;
    }

    if (json.contains("objectValue2")) {
        bool valid = true;
        if (json.value("objectValue2").isObject()) {
            if (!m_objectValue2) {
                m_objectValue2 = new SelfLinkedObject();
                m_deleteObjectValue2 = true;
            }
            valid = valid && m_objectValue2->fromEnginioJson(
                        json.value("objectValue2").toObject());
        } else
            valid = false;

        if (!valid)
            return false;
    }

    return true;
}

QString SelfLinkedObject::id() const
{
    return m_id;
}
