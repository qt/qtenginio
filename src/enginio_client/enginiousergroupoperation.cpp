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

#include "enginiousergroupoperation_p.h"
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

/*!
 * \class EnginioUsergroupOperation
 * \internal
 * \inmodule enginio-client
 * \brief Operation for adding and removing usergroup members.
 *
 * Use this operation to add or remove users to/from usergroup. For
 * getting members of a usergroup use \l EnginioQueryOperation (specifically
 * \l EnginioQueryOperation::setUsergroupId()). New usergroups (like any
 * other Enginio objects) can be created with \l EnginioObjectOperation.
 *
 * Similar to \l EnginioObjectOperation \c EnginioUsergroupOperation supports
 * automatic updating of object models. E.g. usergroup members can be fetched to
 * \l EnginioObjectModel by setting usergroup ID and model to query operation.
 * Setting same model to \c EnginioUsergroupOperation will add user object to
 * the model when user is added to usergroup and delete user object from the
 * model when member is removed from usergroup.
 *
 * This operation also supports
 * \l {https://engin.io/documentation/rest/parameters/include}
 * {\c include request parameter}. It can be used to specify what data is
 * included in the user objects which are added to object model.
 */


EnginioUsergroupOperationPrivate::EnginioUsergroupOperationPrivate(EnginioUsergroupOperation *op) :
    EnginioOperationPrivate(op),
    m_type(Enginio::NullUsergroupOperation),
    m_user(0),
    m_userId(),
    m_usergroupId(),
    m_userOwned(false),
    m_model(0),
    m_modelIndex(),
    m_requestDataBuffer(0)
{
}

EnginioUsergroupOperationPrivate::~EnginioUsergroupOperationPrivate()
{
    if (m_user && m_userOwned)
        delete m_user;
}

/*!
 * URL path for operation
 */
QString EnginioUsergroupOperationPrivate::requestPath() const
{
    if (m_type == Enginio::NullUsergroupOperation || m_usergroupId.isEmpty())
        return QString();

    return QStringLiteral("/v1/usergroups/%1/members").arg(m_usergroupId);
}

/*!
 * Generate and execute backend request for this operation. \a backendUrl is the
 * base URL (protocol://host:port) for backend. Returns a new QNetworkReply
 * object.
 */
QNetworkReply * EnginioUsergroupOperationPrivate::doRequest(const QUrl &backendUrl)
{
    Q_Q(EnginioUsergroupOperation);

    QString path = requestPath();
    QString error;

    if (m_type == Enginio::NullUsergroupOperation)
        error = QStringLiteral("Unknown operation type");
    else if (m_userId.isEmpty())
        error = QStringLiteral("Unknown user ID");
    else if (path.isEmpty())
        error = QStringLiteral("Request URL creation failed");

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

    QJsonObject member;
    member.insert(QStringLiteral("id"), m_userId);
    member.insert(QStringLiteral("objectType"), QStringLiteral("users"));
    QByteArray data;

    switch (m_type) {
    case Enginio::AddMemberOperation:
        return netManager->post(req, QJsonDocument(member).toJson());
    case Enginio::RemoveMemberOperation:
        data = QJsonDocument(member).toJson();
        m_requestDataBuffer = new QBuffer(this);
        m_requestDataBuffer->setData(data);
        m_requestDataBuffer->open(QIODevice::ReadOnly);
        return netManager->sendCustomRequest(req, QByteArray("DELETE"),
                                             m_requestDataBuffer);
    default:
        return 0;
    }
}

void EnginioUsergroupOperationPrivate::handleResults()
{
    if (m_requestDataBuffer) {
         m_requestDataBuffer->close();
         m_requestDataBuffer = 0;
    }

    if (m_type == Enginio::RemoveMemberOperation) {
        if (m_model) {
            QModelIndex index = m_model->indexFromId(m_userId);
            if (index.isValid())
                m_model->removeFromModel(index.row(), 1);
        }
        return;
    }

    QByteArray data = m_reply->readAll();

    qDebug() << "=== Reply" << q_ptr << m_reply->operation() << m_reply->url();
    qDebug() << data;
    qDebug() << "=== Reply end ===";

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        setError(EnginioError::RequestError, QStringLiteral("Invalid reply data"));
        return;
    }

    QJsonObject json = doc.object();
    QString id = json.value(QStringLiteral("id")).toString();
    QString objectType = json.value(QStringLiteral("objectType")).toString();

    if (id.isEmpty()) {
        setError(EnginioError::InternalError,
                 QStringLiteral("Unknown ID on returned object"));
        return;
    } else {
        m_userId = id;
    }

    if (objectType != QStringLiteral("users") ) {
        setError(EnginioError::InternalError,
                 QStringLiteral("Invalid object type on returned object"));
        return;
    }
    if (!m_userId.isEmpty() && m_userId != id) {
        setError(EnginioError::InternalError,
                 QStringLiteral("Object IDs in request and reply do not match"));
        return;
    }

    if (m_user) {
        if (!m_user->fromEnginioJson(json)) {
            setError(EnginioError::RequestError,
                     QStringLiteral("Invalid reply object"));
            return;
        }
        emit userUpdated();
    }

    if (!m_model)
        return;

    EnginioAbstractObject *object = m_client->createObject(objectType, id);
    if (!object)
        return;
    if (!object->fromEnginioJson(json)) {
        delete object;
        setError(EnginioError::RequestError,
                 QStringLiteral("Invalid reply object"));
        return;
    }

    QList<EnginioAbstractObject*> objects;
    objects << object;
    int row = -1;
    if (m_modelIndex.isValid())
        row = m_modelIndex.row();
    m_model->addToModel(objects, row);
}


/*!
 * Create new usergroup operation. \a client must be a valid pointer to
 * \l EnginioClient object. If \a model is specified, user will be
 * added/removed from the model when the operation finishes. \a parent is
 * optional.
 */
EnginioUsergroupOperation::EnginioUsergroupOperation(EnginioClient *client,
                                                     EnginioObjectModel *model,
                                                     QObject *parent) :
    EnginioOperation(client, *new EnginioUsergroupOperationPrivate(this), parent)
{
    qDebug() << this << "created";
    connect(d_ptr, SIGNAL(userUpdated()), this, SIGNAL(userUpdated()));
    if (model)
        setModel(model);
}

/*!
 * Constructor used in inheriting classes.
 */
EnginioUsergroupOperation::EnginioUsergroupOperation(EnginioClient *client,
                                                     EnginioObjectModel *model,
                                                     EnginioUsergroupOperationPrivate &dd,
                                                     QObject *parent) :
    EnginioOperation(client, dd, parent)
{
    qDebug() << this << "created";
    connect(d_ptr, SIGNAL(userUpdated()), this, SIGNAL(userUpdated()));
    if (model)
        setModel(model);
}

/*!
 * Destructor.
 */
EnginioUsergroupOperation::~EnginioUsergroupOperation()
{
    qDebug() << this << "deleted";
}

/*!
 * Get the model set for this operation. If model has not been defined, null
 * pointer will be returned.
 */
EnginioObjectModel * EnginioUsergroupOperation::model() const
{
    Q_D(const EnginioUsergroupOperation);
    return d->m_model;
}

/*!
 * Define \a model that is updated when the operation finishes. Make sure model is
 * not deleted while operation is active.
 */
void EnginioUsergroupOperation::setModel(EnginioObjectModel *model)
{
    Q_D(EnginioUsergroupOperation);
    d->m_model = model;
}

/*!
 * Get model index. It is the position at which users are added to the model.
 */
QModelIndex EnginioUsergroupOperation::modelIndex() const
{
    Q_D(const EnginioUsergroupOperation);
    return d->m_modelIndex;
}

/*!
 * Set model \a index to add users into the model. By default, users are added
 * to the end of the model.
 */
void EnginioUsergroupOperation::setModelIndex(QModelIndex index)
{
    Q_D(EnginioUsergroupOperation);
    d->m_modelIndex = index;
}

/*!
 * Returns the ID of the user added/removed by this operation.
 */
QString EnginioUsergroupOperation::userId() const
{
    Q_D(const EnginioUsergroupOperation);
    return d->m_userId;
}

/*!
 * Returns the ID of the usergroup modified by this operation.
 */
QString EnginioUsergroupOperation::usergroupId() const
{
    Q_D(const EnginioUsergroupOperation);
    return d->m_usergroupId;
}

/*!
 * Set operation to add \a user to usergroup with \a usergroupId. \a user must
 * have valid \c id property. All other properties will be ignored. After
 * a successful operation, \a user is updated to contain full user object.
 *
 * If operation's \c model property is defined, updated \a user object is added
 * to the model at position specified in \c modelIndex property.
 */
void EnginioUsergroupOperation::addMember(EnginioAbstractObject *user,
                                          const QString &usergroupId)
{
    qDebug() << Q_FUNC_INFO << usergroupId;
    Q_D(EnginioUsergroupOperation);
    d->m_type = Enginio::AddMemberOperation;
    d->m_userId = user->id();
    d->m_user = user;
    d->m_usergroupId = usergroupId;
}

/*!
 * Set operation to add \a user to usergroup with \a usergroupId. \a user must
 * have valid \c id property. All other properties will be ignored.
 *
 * If operation's \c model property is defined, updated \a user object is added
 * to the model at position specified in \c modelIndex property.
 */
void EnginioUsergroupOperation::addMember(const EnginioAbstractObject &user,
                                          const QString &usergroupId)
{
    qDebug() << Q_FUNC_INFO << usergroupId;
    Q_D(EnginioUsergroupOperation);
    d->m_type = Enginio::AddMemberOperation;
    d->m_userId = user.id();
    d->m_user = d->m_client->createObject(QStringLiteral("users"));
    d->m_user->fromEnginioJson(
                QJsonDocument::fromJson(user.toEnginioJson()).object());
    d->m_userOwned = true;
    d->m_usergroupId = usergroupId;
}

/*!
 * Set operation to add user with \a userId to usergroup with \a usergroupId.
 *
 * If operation's \c model property is defined, updated user object with
 * \a userId is added to the model at position specified in \c modelIndex
 * property.
 */
void EnginioUsergroupOperation::addMember(const QString &userId,
                                          const QString &usergroupId)
{
    qDebug() << Q_FUNC_INFO << userId << usergroupId;
    Q_D(EnginioUsergroupOperation);
    d->m_type = Enginio::AddMemberOperation;
    d->m_userId = userId;
    d->m_usergroupId = usergroupId;
}

/*!
 * Set operation to remove \a user from usergroup with \a usergroupId. \a user
 * must have valid \c id property. All other properties will be ignored.
 *
 * If operation's \c model property is defined, corresponding user object is
 * also removed from the model.
 */
void EnginioUsergroupOperation::removeMember(EnginioAbstractObject *user,
                                             const QString &usergroupId)
{
    qDebug() << Q_FUNC_INFO << usergroupId;
    Q_D(EnginioUsergroupOperation);
    d->m_type = Enginio::RemoveMemberOperation;
    d->m_userId = user->id();
    d->m_user = user;
    d->m_usergroupId = usergroupId;
}

/*!
 * Set operation to remove \a user from usergroup with \a usergroupId. \a user
 * must have valid \c id property. All other properties will be ignored.
 *
 * If operation's \c model property is defined, corresponding user object is
 * also removed from the model.
 */
void EnginioUsergroupOperation::removeMember(const EnginioAbstractObject &user,
                                             const QString &usergroupId)
{
    qDebug() << Q_FUNC_INFO << usergroupId;
    Q_D(EnginioUsergroupOperation);
    d->m_type = Enginio::RemoveMemberOperation;
    d->m_userId = user.id();
    d->m_usergroupId = usergroupId;
}

/*!
 * Set operation to remove user with \a userId from usergroup with
 * \a usergroupId.
 *
 * If operation's \c model property is defined, corresponding user object is
 * also removed from the model.
 */
void EnginioUsergroupOperation::removeMember(const QString &userId,
                                             const QString &usergroupId)
{
    qDebug() << Q_FUNC_INFO << userId << usergroupId;
    Q_D(EnginioUsergroupOperation);
    d->m_type = Enginio::RemoveMemberOperation;
    d->m_userId = userId;
    d->m_usergroupId = usergroupId;
}
