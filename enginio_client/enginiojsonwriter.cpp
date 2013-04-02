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

#include "enginiojsonwriter_p.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

void EnginioJsonWriter::objectToJson(const QJsonObject &o,
                                     QByteArray &json,
                                     bool isObjectRef)
{
    json += '{';

    if (isObjectRef) {
        QString id = o.value("id").toString();
        QString objectType = o.value("objectType").toString();

        if (!id.isEmpty()) {
            json += "\"id\":\"";
            json += id;
            json += '"';
            if (!objectType.isEmpty())
                json += ',';
        }

        if (!objectType.isEmpty()) {
            json += "\"objectType\":\"";
            json += objectType;
            json += '"';
        }
    } else {
        QJsonObject::ConstIterator i = o.constBegin();
        while (i != o.constEnd()) {
            json += '"';
            json += i.key();
            json += "\":";
            valueToJson(i.value(), json);
            i++;
            if (i != o.constEnd())
                json += ',';
        }
    }

    json += '}';
}

void EnginioJsonWriter::arrayToJson(const QJsonArray &a, QByteArray &json)
{
    json += '[';

    QJsonArray::ConstIterator i = a.constBegin();
    while (i != a.constEnd()) {
        valueToJson(*i, json);
        i++;
        if (i != a.constEnd())
            json += ',';
    }

    json += ']';
}

void EnginioJsonWriter::valueToJson(const QJsonValue &v, QByteArray &json)
{
    switch (v.type()) {
    case QJsonValue::Bool:
        json += v.toBool() ? "true" : "false";
        break;
    case QJsonValue::Double:
        json += QByteArray::number(v.toDouble(), 'f');
        break;
    case QJsonValue::String:
        json += '"';
        json += v.toString();
        json += '"';
        break;
    case QJsonValue::Array:
        arrayToJson(v.toArray(), json);
        break;
    case QJsonValue::Object:
        objectToJson(v.toObject(), json, false);
        break;
    default:
        break;
    }
}
