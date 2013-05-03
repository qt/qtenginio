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

#ifndef ENGINIOJSONOBJECT_H
#define ENGINIOJSONOBJECT_H

#include "enginioabstractobject.h"
#include "enginioclient_global.h"
#include <QJsonObject>

class ENGINIOCLIENT_EXPORT EnginioJsonObject : public EnginioAbstractObject
{
public:
    explicit EnginioJsonObject(const QString &objectType = QString());
    ~EnginioJsonObject();
    void insert(const QString &key, const QJsonValue &value);
    void remove(const QString &key);
    QJsonValue value(const QString &key) const;

    // From EnginioAbstractObject
    QByteArray toEnginioJson(bool isObjectRef = false) const;
    bool fromEnginioJson(const QJsonObject &json);
    QString id() const;
    QString objectType() const;

private:
    QJsonObject m_object;
    Q_DISABLE_COPY(EnginioJsonObject)
};

#endif // ENGINIOJSONOBJECT_H
