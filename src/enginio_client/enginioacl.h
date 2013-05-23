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

#ifndef ENGINIOACL_H
#define ENGINIOACL_H

#include "enginioclient_global.h"
#include <QObject>
#include <QJsonArray>

class EnginioAclPrivate;

class ENGINIOCLIENT_EXPORT EnginioAcl : public QObject
{
    Q_OBJECT
    Q_ENUMS(Permission)

    Q_PROPERTY(QJsonArray readPermissions READ readJson WRITE setReadJson)
    Q_PROPERTY(QJsonArray updatePermissions READ updateJson WRITE setUpdateJson)
    Q_PROPERTY(QJsonArray deletePermissions READ deleteJson WRITE setDeleteJson)
    Q_PROPERTY(QJsonArray adminPermissions READ adminJson WRITE setAdminJson)

public:
    enum Permission {
        ReadPermission = 0,
        UpdatePermission,
        DeletePermission,
        AdminPermission
    };

    explicit EnginioAcl(QObject *parent = 0);
    EnginioAcl(const QJsonObject &object, QObject *parent = 0);
    ~EnginioAcl();

    bool fromJson(const QByteArray &json);
    QByteArray toJson() const;
    bool isSubjectHavingPermission(QPair<QString, QString> subject,
                                   EnginioAcl::Permission permission) const;
    const QList< QPair<QString, QString> >
    getSubjectsForPermission(EnginioAcl::Permission permission) const;

    void addSubjectForPermission(QPair<QString, QString> subject,
                                 EnginioAcl::Permission permission);
    bool removeSubjectFromPermission(QPair<QString, QString> subject,
                                     EnginioAcl::Permission permission);
    void clear();

    QJsonArray readJson() const;
    void setReadJson(const QJsonArray &json);
    QJsonArray updateJson() const;
    void setUpdateJson(const QJsonArray &json);
    QJsonArray deleteJson() const;
    void setDeleteJson(const QJsonArray &json);
    QJsonArray adminJson() const;
    void setAdminJson(const QJsonArray &json);

protected:
    EnginioAclPrivate *d_ptr;
    EnginioAcl(EnginioAclPrivate &dd,
               QObject *parent = 0);

    static QJsonObject objectPairToJson(QPair<QString, QString> pair);
    static QPair<QString, QString> objectJsonToPair(QJsonObject json);
    static QJsonArray permissionListToArray(QList< QPair<QString, QString> > list);
    static QList< QPair<QString, QString> > permissionArrayToList(QJsonArray array);
    static QJsonArray permissionObjectListToArray(QList<QJsonObject> list);
    static QList<QJsonObject> permissionArrayToObjectList(QJsonArray array);

private:
    Q_DECLARE_PRIVATE(EnginioAcl)
    Q_DISABLE_COPY(EnginioAcl)
    friend class EnginioAclOperation;
    friend class EnginioQmlAclOperation;
};

Q_DECLARE_METATYPE(EnginioAcl*)

#endif // ENGINIOACL_H
