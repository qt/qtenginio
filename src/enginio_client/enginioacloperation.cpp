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

#include "enginioacloperation_p.h"

#include <QBuffer>
#include <QDebug>

/*!
 * \class EnginioAclOperation
 * \inmodule enginio-client
 * \brief Operation for fetching and modifying Enginio permissions.
 *
 * Usage:
 * \list
 *     \li Create EnginioAclOperation object.
 *     \li Define Controlled Object (i.e. which object's permissions operation
 *         reads or modifies) with setObject().
 *     \li Define desired actions by calling grantPermission(),
 *         withdrawPermission() or readPermissions().
 *     \li Execute operation by calling execute().
 *     \li After \c finished signal is emitted, get the read/modified
 *         permissions by calling resultAcl().
 * \endlist
 *
 * Note that operation can only act on single object and can be used to
 * grant OR withdraw read permissions. For example following is ok:
 * \code
 *     // Add read and update permissions to object "p1" for users "1" and "2"
 *     aclOperation->setObject(qMakePair("p1", "objects.places"));
 *     aclOperation->grantPermission(qMakePair("1", "users"),
 *                                   EnginioACL::ReadPermission);
 *     aclOperation->grantPermission(qMakePair("1", "users"),
 *                                   EnginioACL::UpdatePermission);
 *     aclOperation->grantPermission(qMakePair("2", "users"),
 *                                   EnginioACL::ReadPermission);
 *     aclOperation->grantPermission(qMakePair("2", "users"),
 *                                   EnginioACL::UpdatePermission);
 *     aclOperation->execute();
 * \endcode
 *
 * But the following is not allowed:
 * \code
 *     // Add read permission and remove delete permission to object "p1" for
 *     // user "1"
 *     aclOperation->setObject(qMakePair("p1", "objects.places"));
 *     aclOperation->grantPermission(qMakePair("1", "users"),
 *                                   EnginioACL::ReadPermission);
 *     aclOperation->withdrawPermission(qMakePair("1", "users"),
 *                                      EnginioACL::DeletePermission);
 *     aclOperation->execute();
 * \endcode
 */

EnginioAclOperationPrivate::EnginioAclOperationPrivate(EnginioAclOperation *op) :
    EnginioOperationPrivate(op),
    m_type(ReadAclOperation),
    m_object(),
    m_requestAcl(),
    m_resultAcl(),
    m_requestDataBuffer(0)
{
}

EnginioAclOperationPrivate::~EnginioAclOperationPrivate()
{
    m_requestAcl.clear();
    m_resultAcl.clear();
}

QString EnginioAclOperationPrivate::requestPath() const
{
    if (m_object.first.isEmpty() || m_object.second.isEmpty())
        return QString();

    // If object type starts with "objects.", url should be "/v1/objects/type".
    // If not, url should be /v1/type.
    QString objectType(m_object.second);
    if (objectType.startsWith(QStringLiteral("objects.")))
        objectType[7] = QChar('/');

    return QString("/v1/%1/%2/access").arg(objectType).arg(m_object.first);
}

QNetworkReply * EnginioAclOperationPrivate::doRequest(const QUrl &backendUrl)
{
    Q_Q(EnginioAclOperation);

    QString path = requestPath();
    QString error;

    if (m_object.first.isEmpty())
        error = "Unknown object ID";
    if (m_object.second.isEmpty())
        error = "Unknown object type";
    else if (path.isEmpty())
        error = "Request URL creation failed";

    if (!error.isEmpty()) {
        setError(EnginioError::RequestError, error);
        emit q->finished();
        return 0;
    }

    QUrl url(backendUrl);
    url.setPath(path);
    url.setQuery(urlQuery());

    QNetworkRequest req = enginioRequest(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                  QStringLiteral("application/json"));
    QNetworkAccessManager *netManager = m_client->networkManager();

    switch (m_type) {
    case AddAclOperation:
        return netManager->put(req, m_requestAcl->toJson());
        return 0;
    case DeleteAclOperation:
        m_requestData = m_requestAcl->toJson();
        m_requestDataBuffer = new QBuffer(&m_requestData, q_ptr);
        m_requestDataBuffer->open(QIODevice::ReadOnly);
        return netManager->sendCustomRequest(req, QByteArray("DELETE"),
                                             m_requestDataBuffer);
    case UpdateAclOperation:
        return netManager->post(req, m_requestAcl->toJson());
    case ReadAclOperation:
        return netManager->get(req);
    default:
        return 0;
    }
}

void EnginioAclOperationPrivate::handleResults()
{
    if (m_requestDataBuffer)
        m_requestDataBuffer->close();

    QByteArray data = m_reply->readAll();

    qDebug() << "=== Reply" << q_ptr << m_reply->operation() << m_reply->url();
    qDebug() << data;
    qDebug() << "=== Reply end ===";

    if (m_resultAcl.isNull()) {
        m_resultAcl = QSharedPointer<EnginioAcl>(new EnginioAcl());
    }

    if (!m_resultAcl->fromJson(data)) {
        setError(EnginioError::RequestError, "Invalid reply data");
    }
}


/*!
 * Create new ACL operation. \a client must be valid pointer to EnginioClient
 * object. \a parent is optional parent QObject.
 */
EnginioAclOperation::EnginioAclOperation(EnginioClient *client,
                                         QObject *parent) :
    EnginioOperation(client, *new EnginioAclOperationPrivate(this), parent)
{
    qDebug() << this << "EnginioAclOperation created";
}

/*!
 * Constructor used in inheriting classes.
 */
EnginioAclOperation::EnginioAclOperation(EnginioClient *client,
                                         EnginioAclOperationPrivate &dd,
                                         QObject *parent) :
    EnginioOperation(client, dd, parent)
{
    qDebug() << this << "created";
}

/*!
 * Destructor.
 */
EnginioAclOperation::~EnginioAclOperation()
{
    qDebug() << this << "deleted";
}

/*!
 * Returns Controlled Object as <ID, objectType> pair.
 */
QPair<QString, QString> EnginioAclOperation::object() const
{
    Q_D(const EnginioAclOperation);
    return d->m_object;
}

/*!
 * Set Controlled Object for operation. \a object must be given as
 * <ID, objectType> pair.
 */
void EnginioAclOperation::setObject(QPair<QString, QString> object)
{
    Q_D(EnginioAclOperation);
    d->m_object = object;
}

/*!
 * Returns permission operation results. If result object has been set with
 * \l setResultAcl() this method returns same object. If result object has not
 * been set and operation has finished newly created object is returned.
 *
 * De-reference returned object with \c QSharedPointer::clear() when it is no
 * longer used.
 */
QSharedPointer<EnginioAcl> EnginioAclOperation::resultAcl()
{
    Q_D(const EnginioAclOperation);
    return d->m_resultAcl;
}

/*!
 * Sets operation result object which will be updated when operation finishes.
 */
void EnginioAclOperation::setResultAcl(QSharedPointer<EnginioAcl> resultAcl)
{
    Q_D(EnginioAclOperation);
    d->m_resultAcl = resultAcl;
}

/*!
 * Grants to \a subject a \a permission for Controlled Object. Permission allows
 * the subject to perform certain operations for object in question.
 *
 * Subject can be:
 * \list
 *    \li User
 *    \li Usergroup
 *    \li Well known subject
 * \endlist
 *
 * Possible permissions are:
 * \list
 *    \li "read"
 *    \li "update"
 *    \li "delete"
 *    \li "admin"
 * \endlist
 *
 * Controlled Object can be any Enginio Object. Set it with \l setObject().
 *
 * Authenticated User must have the "admin" permission for the Controlled Object
 * or otherwise the operation fails with insufficient permissions error.
 */
void EnginioAclOperation::grantPermission(QPair<QString, QString> subject,
                                          EnginioAcl::Permission permission)
{
    Q_D(EnginioAclOperation);

    if (d->m_requestAcl.isNull())
        d->m_requestAcl = QSharedPointer<EnginioAcl>(new EnginioAcl());

    d->m_type = EnginioAclOperationPrivate::AddAclOperation;
    d->m_requestAcl->addSubjectForPermission(subject, permission);
}

/*!
 * Withdraws from \a subject a \a permission for Controlled Object.
 *
 * Subject can be:
 * \list
 *    \li User
 *    \li Usergroup
 *    \li Well known subject
 * \endlist
 *
 * Possible permissions are:
 * \list
 *    \li "read"
 *    \li "update"
 *    \li "delete"
 *    \li "admin"
 * \endlist
 *
 * Controlled Object can be any Enginio Object. Set it with \l setObject().
 *
 * Authenticated User must have the "admin" permission for the Controlled Object
 * or otherwise the operation fails with insufficient permissions error.
 */
void EnginioAclOperation::withdrawPermission(QPair<QString, QString> subject,
                                             EnginioAcl::Permission permission)
{
    Q_D(EnginioAclOperation);

    if (d->m_requestAcl.isNull())
        d->m_requestAcl = QSharedPointer<EnginioAcl>(new EnginioAcl());

    d->m_type = EnginioAclOperationPrivate::DeleteAclOperation;
    d->m_requestAcl->addSubjectForPermission(subject, permission);
}

/*!
 * Set permissions for Controlled Object. Overwriting previous permissions with
 * \a permissions.
 *
 * Possible permissions are:
 * \list
 *    \li "read"
 *    \li "update"
 *    \li "delete"
 *    \li "admin"
 * \endlist
 *
 * Controlled Object can be any Enginio Object. Set it with \l setObject().
 *
 * Authenticated User must have the "admin" permission for the Controlled Object
 * or otherwise the operation fails with insufficient permissions error.
 */
void EnginioAclOperation::setPermissions(QSharedPointer<EnginioAcl> permissions)
{
    Q_D(EnginioAclOperation);
    d->m_type = EnginioAclOperationPrivate::UpdateAclOperation;
    d->m_requestAcl = permissions;
}

/*!
 * Reads permissions of Controlled Object. Controlled Object can be any Enginio
 * Object. Set it with \l setObject().
 */
void EnginioAclOperation::readPermissions()
{
    Q_D(EnginioAclOperation);
    d->m_type = EnginioAclOperationPrivate::ReadAclOperation;
}

/*!
 * \internal
 * Returns access control list that is sent to server.
 */
QSharedPointer<EnginioAcl> EnginioAclOperation::requestAcl()
{
    Q_D(const EnginioAclOperation);
    return d->m_requestAcl;
}

/*!
 * \internal
 * Sets permission to be granted. Overwrites all permissions set to this
 * operation.
 */
void EnginioAclOperation::setAddPermissions(QSharedPointer<EnginioAcl> acl)
{
    Q_D(EnginioAclOperation);
    d->m_requestAcl = acl;
    d->m_type = EnginioAclOperationPrivate::AddAclOperation;
}

/*!
 * \internal
 * Sets permission to be withdrawn. Overwrites all permissions set to this
 * operation.
 */
void EnginioAclOperation::setDeletePermissions(QSharedPointer<EnginioAcl> acl)
{
    Q_D(EnginioAclOperation);
    d->m_requestAcl = acl;
    d->m_type = EnginioAclOperationPrivate::DeleteAclOperation;
}
