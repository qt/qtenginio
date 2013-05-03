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

#ifndef ENGINIOQUERYOPERATION_H
#define ENGINIOQUERYOPERATION_H

#include "enginiooperation.h"

class EnginioAbstractObject;
class EnginioObjectModel;
class EnginioQueryOperationPrivate;

class ENGINIOCLIENT_EXPORT EnginioQueryOperation : public EnginioOperation
{
    Q_OBJECT
public:
    explicit EnginioQueryOperation(EnginioClient *client,
                                   EnginioObjectModel *model = 0,
                                   QObject *parent = 0);
    virtual ~EnginioQueryOperation();

    QString objectType() const;
    void setObjectType(const QString &objectType);
    EnginioObjectModel * model() const;
    void setModel(EnginioObjectModel *model);
    QModelIndex modelIndex() const;
    void setModelIndex(QModelIndex index);
    QList<EnginioAbstractObject*> takeResults();
    QStringList objectTypes() const;
    void setObjectTypes(const QStringList &objectTypes);
    Q_INVOKABLE void addObjectType(const QString &objectType);

protected:
    EnginioQueryOperation(EnginioClient *client,
                          EnginioObjectModel *model,
                          EnginioQueryOperationPrivate &dd,
                          QObject *parent = 0);

private:
    Q_DECLARE_PRIVATE(EnginioQueryOperation)
    Q_DISABLE_COPY(EnginioQueryOperation)
};

#endif // ENGINIOQUERYOPERATION_H
