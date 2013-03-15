/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://qt.digia.com/contact-us
**
** This file is part of the Enginio Qt Client Library.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
****************************************************************************/

#include "enginiojsonobject.h"
#include "enginiojsonwriter_p.h"
#include <QDebug>

/*!
 * \class EnginioJsonObject
 * \inmodule enginio-client
 * \brief Generic JSON object stored in the Enginio backend.
 *
 * You can use this class if you don't want to define a custom class.
 *
 */

EnginioJsonObject::EnginioJsonObject(const QString &objectType)
{
    qDebug() << this << "EnginioJsonObject created. Type:" << objectType;

    if (!objectType.isEmpty())
        m_object.insert(QStringLiteral("objectType"), objectType);
}

EnginioJsonObject::~EnginioJsonObject()
{
    qDebug() << this << "EnginioJsonObject deleted";
}

/*!
 * Insert new property with a \a key, and a \a value. If there is
 * already a property with same key, old value will be replaced.
 */
void EnginioJsonObject::insert(const QString &key, const QJsonValue &value)
{
    m_object.insert(key, value);
}

/*!
 * Removes property \a key from object.
 */
void EnginioJsonObject::remove(const QString &key)
{
    m_object.remove(key);
}

/*!
 * Returns value of the property, \a key. If property does not exist,
 * returned QJsonValue will be \c Undefined.
 */
QJsonValue EnginioJsonObject::value(const QString &key) const
{
    return m_object.value(key);
}

QByteArray EnginioJsonObject::toEnginioJson(bool isObjectRef) const
{
    QByteArray json;
    EnginioJsonWriter::objectToJson(m_object, json, isObjectRef);
    qDebug() << Q_FUNC_INFO << json;
    return json;
}

bool EnginioJsonObject::fromEnginioJson(const QJsonObject &json)
{
    qDebug() << Q_FUNC_INFO << json;
    QJsonObject::const_iterator i = json.constBegin();
    while (i != json.constEnd()) {
        m_object.insert(i.key(), i.value());
        i++;
    }
    return true;
}

QString EnginioJsonObject::id() const
{
    return value(QStringLiteral("id")).toString();
}

QString EnginioJsonObject::objectType() const
{
    return value(QStringLiteral("objectType")).toString();
}
