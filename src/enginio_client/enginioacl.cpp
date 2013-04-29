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

#include "enginioacl_p.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

/*!
 * \class EnginioAcl
 * \inmodule enginio-client
 * \brief EnginioAcl defines access control list for Enginio objects.
 *
 * Access control list contains a list of subjects and set of permissions
 * granted for each subject.
 *
 * Subjects can be:
 * \list
 *   \li Users
 *   \li Usergroups
 *   \li Well known subjects like "everyone"
 * \endlist
 *
 * And possible permissions in ACL for objects are:
 * \list
 *   \li "read"
 *   \li "update"
 *   \li "delete"
 *   \li "admin"
 * \endlist
 *
 * Subjects are presented as \c {<ID, ObjectType>} pairs and permissions as
 * EnginioAcl::Permission type enumerations.
 *
 * Well known subjects are constants which identify generic Users or Usergroups.
 * For example 'everyone' subject presents all Users (even anonymous) and its
 * value is \c {<"*", "aclSubject">} (that is, ID is "*" and ObjectType is
 * "aclSubject").
 */

/*!
 * \enum EnginioAcl::Permission
 * \value ReadPermission
 *        Permission to read object data.
 * \value UpdatePermission
 *        Permission to update object data.
 * \value DeletePermission
 *        Permission to delete object.
 * \value AdminPermission
 *        Permission to read, update and delete object and to read and change
 *        object permissions.
 */

const char* EnginioAclPrivate::m_permissionNames[] = { "read", "update",
                                                       "delete", "admin" };
const int EnginioAclPrivate::m_numPermissions = 4;

EnginioAclPrivate::EnginioAclPrivate(EnginioAcl *acl) :
    q_ptr(acl),
    m_accessLists(m_numPermissions)
{
}

EnginioAclPrivate::~EnginioAclPrivate()
{
}

/*!
 * Create new EnginioAcl object with optional parent \a parent.
 */
EnginioAcl::EnginioAcl(QObject *parent) :
    QObject(parent),
    d_ptr(new EnginioAclPrivate(this))
{
}

/*!
 * Create new EnginioAcl object with optional parent \a parent and initialize it
 * from \a object. \a object should contain properties for permission names
 * ("read", "update", etc) as arrays of subjects. Subjects are QJsonObjects with
 * "id" and "objectType" properties.
 */
EnginioAcl::EnginioAcl(const QJsonObject &object, QObject *parent) :
    QObject(parent),
    d_ptr(new EnginioAclPrivate(this))
{
    Q_D(EnginioAcl);
    for (int perm = 0; perm < d->m_numPermissions; perm++) {
        if (object.contains(d->m_permissionNames[perm])) {
            d->m_accessLists[perm] = permissionArrayToList(
                        object[d->m_permissionNames[perm]].toArray());
        }
    }
}

/*!
 * Constructor used in inheriting classes.
 */
EnginioAcl::EnginioAcl(EnginioAclPrivate &dd, QObject *parent) :
    QObject(parent),
    d_ptr(&dd)
{
}

/*!
 * Destructor.
 */
EnginioAcl::~EnginioAcl()
{
    delete d_ptr;
}

/*!
 * Initializes ACL from JSON permission data.
 */
bool EnginioAcl::fromJson(const QByteArray &json)
{
    qDebug() << Q_FUNC_INFO << json;
    Q_D(EnginioAcl);

    QJsonParseError *parseError = new QJsonParseError;
    QJsonDocument doc = QJsonDocument::fromJson(json, parseError);

    if (parseError->error != QJsonParseError::NoError) {
        qWarning() << Q_FUNC_INFO << "parse error:" << parseError->errorString();
        delete parseError;
        return false;
    }

    delete parseError;

    QJsonObject jsonObject = doc.object();
    if (jsonObject.isEmpty()) {
        return false;
    }

    bool error = false;

    for (int listItem = 0; listItem < d->m_numPermissions; listItem++) {
        d->m_accessLists[listItem].clear();

        QJsonValue listValue = jsonObject.value(d->m_permissionNames[listItem]);
        if (!listValue.isArray()) {
            error = true;
            continue;
        }

        QJsonArray listArray = listValue.toArray();
        for (int item = 0; item < listArray.size(); item++) {
            QJsonObject itemObject = listArray.at(item).toObject();
            QString id = itemObject.value(QStringLiteral("id")).toString();
            QString type = itemObject.value(QStringLiteral("objectType")).toString();

            if (!id.isEmpty() && !type.isEmpty()) {
                d->m_accessLists[listItem].append(qMakePair(id, type));
            } else {
                error = true;
            }
        }
    }
    return !error;
}

/*!
 * Returns JSON representation of permissions.
 */
QByteArray EnginioAcl::toJson() const
{
    Q_D(const EnginioAcl);

    QByteArray json;
    json += '{';

    for (int i = 0; i < d->m_numPermissions; i++) {
        if (i > 0)
            json += ',';

        json += QByteArray("\"") + d->m_permissionNames[i] + "\":[";

        const QList< QPair<QString, QString> > list = d->m_accessLists.at(i);
        QList< QPair<QString, QString> >::ConstIterator iter = list.constBegin();
        while (iter != list.constEnd()) {
            if (iter != list.constBegin())
                json += ',';

            json += QByteArray("{\"id\":\"") + iter->first + "\",\"objectType\":\"" + iter->second + "\"}";
            iter++;
        }
        json += ']';
    }
    json += '}';

    qDebug() << Q_FUNC_INFO << json;
    return json;
}

/*!
 * Returns true if \a subject is having given \a permission.
 */
bool EnginioAcl::isSubjectHavingPermission(QPair<QString, QString> subject,
                                           EnginioAcl::Permission permission) const
{
    const QList< QPair<QString, QString> > list = getSubjectsForPermission(permission);
    QList< QPair<QString, QString> >::ConstIterator i = list.constBegin();
    while (i != list.constEnd()) {
        if (*i == subject)
            return true;
        i++;
    }

    // Admin permission gives also read, update and delete permissions.
    if (permission == EnginioAcl::ReadPermission ||
        permission == EnginioAcl::UpdatePermission ||
        permission == EnginioAcl::DeletePermission)
    {
        return isSubjectHavingPermission(subject, EnginioAcl::AdminPermission);
    }

    return false;
}

/*!
 * Gets subjects which have given \a permission. Returned list contains zero or
 * more Users, Usergroups or well known subjects as <ID, objectType> pairs.
 */
const QList< QPair<QString, QString> >
EnginioAcl::getSubjectsForPermission(EnginioAcl::Permission permission) const
{
    Q_D(const EnginioAcl);
    return d->m_accessLists.at((int)permission);
}

/*!
 * Adds \a subject to permission list of \a permission.
 */
void EnginioAcl::addSubjectForPermission(QPair<QString, QString> subject,
                                         EnginioAcl::Permission permission)
{
    Q_D(EnginioAcl);
    if (!d->m_accessLists.at((int)permission).contains(subject))
        d->m_accessLists[(int)permission].append(subject);
}

/*!
 * Removes \a subject from permission list of \a permission. Returns \c true if
 * subject was removed.
 */
bool EnginioAcl::removeSubjectFromPermission(QPair<QString, QString> subject,
                                             EnginioAcl::Permission permission)
{
    Q_D(EnginioAcl);
    int removed = 0;
    if (d->m_accessLists.at((int)permission).contains(subject))
        removed = d->m_accessLists[(int)permission].removeAll(subject);

    return removed > 0;
}

/*!
 * Deletes all subjects from all permissions.
 */
void EnginioAcl::clear()
{
    Q_D(EnginioAcl);
    for (int i = 0; i < d->m_numPermissions; i++) {
        d->m_accessLists[i].clear();
    }
}

/*!
 * Returns subjects in "read" access list. Returned array contains subjects as
 * QJsonObjects with \c id and \c objectType properties.
 */
QJsonArray EnginioAcl::readJson() const
{
    Q_D(const EnginioAcl);
    return permissionListToArray(d->m_accessLists.at(ReadPermission));
}

/*!
 * Sets subjects in "read" access list to be same as in \a json array. \a json
 * array should contain subjects as QJsonObjects with \c id and \c objectType
 * properties set. Existing subjects in "read" list will be overwritten.
 */
void EnginioAcl::setReadJson(const QJsonArray &json)
{
    Q_D(EnginioAcl);
    d->m_accessLists[ReadPermission] = permissionArrayToList(json);
}

/*!
 * Returns subjects in "update" access list. Returned array contains subjects as
 * QJsonObjects with \c id and \c objectType properties.
 */
QJsonArray EnginioAcl::updateJson() const
{
    Q_D(const EnginioAcl);
    return permissionListToArray(d->m_accessLists.at(UpdatePermission));
}

/*!
 * Sets subjects in "update" access list to be same as in \a json array. \a json
 * array should contain subjects as QJsonObjects with \c id and \c objectType
 * properties set. Existing subjects in "update" list will be overwritten.
 */
void EnginioAcl::setUpdateJson(const QJsonArray &json)
{
    Q_D(EnginioAcl);
    d->m_accessLists[UpdatePermission] = permissionArrayToList(json);
}

/*!
 * Returns subjects in "delete" access list. Returned array contains subjects as
 * QJsonObjects with \c id and \c objectType properties.
 */
QJsonArray EnginioAcl::deleteJson() const
{
    Q_D(const EnginioAcl);
    return permissionListToArray(d->m_accessLists.at(DeletePermission));
}

/*!
 * Sets subjects in "delete" access list to be same as in \a json array. \a json
 * array should contain subjects as QJsonObjects with \c id and \c objectType
 * properties set. Existing subjects in "delete" list will be overwritten.
 */
void EnginioAcl::setDeleteJson(const QJsonArray &json)
{
    Q_D(EnginioAcl);
    d->m_accessLists[DeletePermission] = permissionArrayToList(json);
}

/*!
 * Returns subjects in "admin" access list. Returned array contains subjects as
 * QJsonObjects with \c id and \c objectType properties.
 */
QJsonArray EnginioAcl::adminJson() const
{
    Q_D(const EnginioAcl);
    return permissionListToArray(d->m_accessLists.at(AdminPermission));
}

/*!
 * Sets subjects in "admin" access list to be same as in \a json array. \a json
 * array should contain subjects as QJsonObjects with \c id and \c objectType
 * properties set. Existing subjects in "admin" list will be overwritten.
 */
void EnginioAcl::setAdminJson(const QJsonArray &json)
{
    Q_D(EnginioAcl);
    d->m_accessLists[AdminPermission] = permissionArrayToList(json);
}

/*!
 * \internal
 * Converts Enginio object represented as <ID, objectType> pair to QJsonObject.
 */
QJsonObject EnginioAcl::objectPairToJson(QPair<QString, QString> pair)
{
    QJsonObject json;
    json.insert(QStringLiteral("id"), pair.first);
    json.insert(QStringLiteral("objectType"), pair.second);
    return json;
}

/*!
 * \internal
 * Converts Enginio object represented as QJsonObject to <ID, objectType> pair.
 */
QPair<QString, QString> EnginioAcl::objectJsonToPair(QJsonObject json)
{
    QPair<QString, QString> pair;
    pair.first = json.value(QStringLiteral("id")).toString();
    pair.second = json.value(QStringLiteral("objectType")).toString();
    return pair;
}

/*!
 * \internal
 * Converts Enginio object list represented as QList of <ID, objectType> pairs
 * to QJsonArray of QJsonObjects.
 */
QJsonArray EnginioAcl::permissionListToArray(QList< QPair<QString, QString> > list)
{
    QJsonArray array;
    for (int i = 0; i < list.size(); i++) {
        array.append(objectPairToJson(list.at(i)));
    }
    return array;
}

/*!
 * \internal
 * Converts Enginio object list represented as QJsonArray of QJsonObjects to
 * QList of <ID, objectType> pairs.
 */
QList< QPair<QString, QString> > EnginioAcl::permissionArrayToList(QJsonArray array)
{
    QList< QPair<QString, QString> > list;
    QJsonArray::const_iterator iter = array.constBegin();
    while (iter != array.constEnd()) {
        list.append(objectJsonToPair((*iter).toObject()));
        iter++;
    }
    return list;
}
