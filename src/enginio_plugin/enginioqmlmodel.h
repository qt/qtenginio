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

#ifndef ENGINIOQMLMODEL_H
#define ENGINIOQMLMODEL_H

#include <Enginio/enginiomodelbase.h>
#include <QtQml/qjsvalue.h>

QT_BEGIN_NAMESPACE

class EnginioQmlReply;
class EnginioQmlClient;

class EnginioQmlModel : public EnginioModelBase
{
    Q_OBJECT
    Q_DISABLE_COPY(EnginioQmlModel)
public:
    EnginioQmlModel(QObject *parent = 0);
    ~EnginioQmlModel();

    Q_PROPERTY(EnginioQmlClient *enginio READ enginio WRITE setEnginio NOTIFY enginioChanged)
    Q_PROPERTY(QJSValue query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

    // TODO: that is a pretty silly name
    EnginioQmlClient *enginio() const Q_REQUIRED_RESULT;
    void setEnginio(const EnginioQmlClient *enginio);

    QJSValue query() Q_REQUIRED_RESULT;
    void setQuery(const QJSValue &query);

    Q_INVOKABLE EnginioQmlReply *append(const QJSValue &value);
    Q_INVOKABLE EnginioQmlReply *remove(int row);
    Q_INVOKABLE EnginioQmlReply *setProperty(int row, const QString &role, const QVariant &value);

Q_SIGNALS:
    void queryChanged(const QJSValue &query);
    void enginioChanged(EnginioQmlClient *enginio);
    void rowCountChanged();
};

QT_END_NAMESPACE

#endif // ENGINIOQMLOBJECTMODEL_H
