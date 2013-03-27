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

#ifndef ENGINIOACLOPERATION_H
#define ENGINIOACLOPERATION_H

#include "enginioacl.h"
#include "enginiooperation.h"

class EnginioAclOperationPrivate;

class ENGINIOCLIENT_EXPORT EnginioAclOperation : public EnginioOperation
{
    Q_OBJECT

public:
    explicit EnginioAclOperation(EnginioClient *client,
                                 QObject *parent = 0);
    virtual ~EnginioAclOperation();

    QPair<QString, QString> object() const;
    void setObject(QPair<QString, QString> object);
    QSharedPointer<EnginioAcl> resultAcl();
    void setResultAcl(QSharedPointer<EnginioAcl> resultAcl);

    void grantPermission(QPair<QString, QString> subject,
                         EnginioAcl::Permission permission);
    void withdrawPermission(QPair<QString, QString> subject,
                            EnginioAcl::Permission permission);
    void setPermissions(QSharedPointer<EnginioAcl> permissions);
    Q_INVOKABLE void readPermissions();

protected:
    EnginioAclOperation(EnginioClient *client,
                        EnginioAclOperationPrivate &dd,
                        QObject *parent = 0);

    QSharedPointer<EnginioAcl> requestAcl();
    void setAddPermissions(QSharedPointer<EnginioAcl> acl);
    void setDeletePermissions(QSharedPointer<EnginioAcl> acl);

private:
    Q_DECLARE_PRIVATE(EnginioAclOperation)
};

#endif // ENGINIOACLOPERATION_H
