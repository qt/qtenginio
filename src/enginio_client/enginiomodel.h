/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtEnginio module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
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
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef ENGINIOMODEL_H
#define ENGINIOMODEL_H

#include <QtCore/qjsonobject.h>
#include <QtCore/qscopedpointer.h>

#include <Enginio/enginioclient.h>
#include <Enginio/enginiomodelbase.h>

QT_BEGIN_NAMESPACE

class EnginioModelBasePrivate;
class ENGINIOCLIENT_EXPORT EnginioModel : public EnginioModelBase
{
    Q_OBJECT
public:
    explicit EnginioModel(QObject *parent = 0);
    ~EnginioModel();

    Q_PROPERTY(EnginioClient *enginio READ enginio WRITE setEnginio NOTIFY enginioChanged)
    Q_PROPERTY(QJsonObject query READ query WRITE setQuery NOTIFY queryChanged)

    // TODO: that is a pretty silly name
    EnginioClient *enginio() const Q_REQUIRED_RESULT;
    void setEnginio(const EnginioClient *enginio);

    QJsonObject query() Q_REQUIRED_RESULT;
    void setQuery(const QJsonObject &query);

    Q_INVOKABLE EnginioReply *append(const QJsonObject &value);
    Q_INVOKABLE EnginioReply *remove(int row);
    Q_INVOKABLE EnginioReply *setProperty(int row, const QString &role, const QVariant &value);

Q_SIGNALS:
    void queryChanged(const QJsonObject &query);
    void enginioChanged(EnginioClient *enginio);

private:
    Q_DISABLE_COPY(EnginioModel)
    friend class EnginioModelBasePrivate;
};

QT_END_NAMESPACE

#endif // ENGINIOMODEL_H
