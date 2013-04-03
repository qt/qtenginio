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

#include "enginioobjectoperation_p.h"
#include <QByteArray>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

/*!
 * \class EnginioObjectOperation
 * \inmodule enginio-client
 * \brief Operation acting on a single object
 *
 * Usage:
 * \list
 *      \li Get or create a new object
 *      \li Create object operation
 *      \li (optional) define a model which contains an object that should
 *          be updated or a newly created object should be added.
 *      \li Define the operation type by calling one of the functions.
 *          Operation is tied to the object given as argument.
 *      \li Execute operation
 *      \li After \c finished signal, possibly changed properties can
 *          be read from the original object or from the specified
 *          model.
 * \endlist
 */


EnginioObjectOperationPrivate::EnginioObjectOperationPrivate(EnginioObjectOperation *op) :
    EnginioOperationPrivate(op),
    m_type(Enginio::NullObjectOperation),
    m_object(0),
    m_objectId(),
    m_objectType(),
    m_objectOwned(false),
    m_model(0),
    m_modelIndex()
{
}

EnginioObjectOperationPrivate::~EnginioObjectOperationPrivate()
{
    if (m_object && m_objectOwned)
        delete m_object;
}

/*!
 * URL path for operation
 */
QString EnginioObjectOperationPrivate::requestPath() const
{
    if (m_type == Enginio::NullObjectOperation || m_objectType.isEmpty())
        return QString();

    // If object type starts with "objects.", url should be "/v1/objects/type".
    // If not, url should be /v1/type.
    QString objectType(m_objectType);
    if (objectType.startsWith(QStringLiteral("objects.")))
        objectType[7] = QChar('/');

    if (m_type == Enginio::CreateObjectOperation)
        return "/v1/" + objectType;

    if (m_objectId.isEmpty())
        return QString();

    return "/v1/" + objectType + "/" + m_objectId;
}

/*!
 * Generate and execute backend request for this operation. \a backendUrl is the
 * base URL (protocol://host:port) for backend. Returns a new QNetworkReply
 * object.
 */
QNetworkReply * EnginioObjectOperationPrivate::doRequest(const QUrl &backendUrl)
{
    QString path = requestPath();
    QString error;

    if (m_type == Enginio::NullObjectOperation)
        error = "Unknown operation type";
    else if (m_objectType.isEmpty())
        error = "Unknown object type";
    else if (m_type != Enginio::CreateObjectOperation && m_objectId.isEmpty())
        error = "Unknown object ID";
    else if (path.isEmpty())
        error = "Request URL creation failed";

    if (!error.isEmpty()) {
        setError(EnginioError::RequestError, error);
        emit finished();
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
    case Enginio::CreateObjectOperation:
        return netManager->post(req, m_object->toEnginioJson());
    case Enginio::ReadObjectOperation:
        return netManager->get(req);
    case Enginio::UpdateObjectOperation:
        return netManager->put(req, m_object->toEnginioJson());
    case Enginio::RemoveObjectOperation:
        return netManager->deleteResource(req);
    default:
        return 0;
    }
}

void EnginioObjectOperationPrivate::handleResults()
{
    if (m_type == Enginio::RemoveObjectOperation) {
        if (m_model) {
            QModelIndex index = m_model->indexFromId(m_objectId);
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
    }
    if (objectType.isEmpty()) {
        setError(EnginioError::InternalError,
                 QStringLiteral("Unknown type on returned object"));
        return;
    }
    if (!m_objectId.isEmpty() && m_objectId != id) {
        setError(EnginioError::InternalError,
                 QStringLiteral("Object IDs in request and reply do not match"));
        return;
    }

    if (m_object && !m_objectOwned) {
        if (!m_object->fromEnginioJson(json)) {
            setError(EnginioError::RequestError,
                     QStringLiteral("Invalid reply object"));
            return;
        }
        emit objectUpdated();
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

    if (m_type == Enginio::CreateObjectOperation) {
        QList<EnginioAbstractObject*> objects;
        objects << object;
        int row = -1;
        if (m_modelIndex.isValid())
            row = m_modelIndex.row();
        m_model->addToModel(objects, row);
    } else { // Read and Update operations
        QModelIndex index = m_model->indexFromId(id);
        if (index.isValid()) {
            m_model->setData(index, QVariant::fromValue(object),
                             Enginio::DataRole);
            delete object;
        } else {
            qWarning() << "Failed to find object from model";
        }
    }
}


/*!
 * Create new object operation. \a client must be a valid pointer to the EnginioClient
 * object. If \a model is specified, object will be added/updated/removed from
 * the model when the operation finishes. \a parent is optional.
 */
EnginioObjectOperation::EnginioObjectOperation(EnginioClient *client,
                                               EnginioObjectModel *model,
                                               QObject *parent) :
    EnginioOperation(client, *new EnginioObjectOperationPrivate(this), parent)
{
    qDebug() << this << "created";
    connect(d_ptr, SIGNAL(objectUpdated()), this, SIGNAL(objectUpdated()));
    if (model)
        setModel(model);
}

/*!
 * Constructor used in inheriting classes.
 */
EnginioObjectOperation::EnginioObjectOperation(EnginioClient *client,
                                               EnginioObjectModel *model,
                                               EnginioObjectOperationPrivate &dd,
                                               QObject *parent) :
    EnginioOperation(client, dd, parent)
{
    qDebug() << this << "created";
    connect(d_ptr, SIGNAL(objectUpdated()), this, SIGNAL(objectUpdated()));
    if (model)
        setModel(model);
}

/*!
 * Destructor.
 */
EnginioObjectOperation::~EnginioObjectOperation()
{
    qDebug() << this << "deleted";
}

/*!
 * Get the model set for this operation. If model has not been defined, null
 * pointer will be returned.
 */
EnginioObjectModel * EnginioObjectOperation::model() const
{
    Q_D(const EnginioObjectOperation);
    return d->m_model;
}

/*!
 * Define \a model that is updated when the operation finishes. Make sure model is
 * not deleted while operation is active.
 */
void EnginioObjectOperation::setModel(EnginioObjectModel *model)
{
    Q_D(EnginioObjectOperation);
    d->m_model = model;
}

/*!
 * Get model index. It is the position at which new objects are added
 * to the model.
 */
QModelIndex EnginioObjectOperation::modelIndex() const
{
    Q_D(const EnginioObjectOperation);
    return d->m_modelIndex;
}

/*!
 * Set model \a index to add new objects into the model. By
 * default, new objects are added to the end of the model.
 */
void EnginioObjectOperation::setModelIndex(QModelIndex index)
{
    Q_D(EnginioObjectOperation);
    d->m_modelIndex = index;
}

/*!
 * Set operation to create \a object in the Enginio backend. \a object must have
 * \c objectType and any custom properties set. After a successful operation,
 * \a object is updated, so make sure it is not deleted before the operation
 * emits the \c finished signal.
 *
 * If operation's \c model property is defined, new object is added to
 * the model at position specified in \c modelIndex property.
 */
void EnginioObjectOperation::create(EnginioAbstractObject *object)
{
    Q_D(EnginioObjectOperation);
    d->m_type = Enginio::CreateObjectOperation;
    d->m_objectType = object->objectType();
    d->m_object = object;
}

/*!
 * Set operation to create \a object on the Enginio backend. \a object must have
 * \c objectType and any custom properties set.
 *
 * If operation's \c model property is defined, new object is added to
 * the model at position specified in the \c modelIndex property.
 */
void EnginioObjectOperation::create(const EnginioAbstractObject &object)
{
    Q_D(EnginioObjectOperation);
    d->m_type = Enginio::CreateObjectOperation;
    d->m_objectType = object.objectType();
    d->m_object = d->m_client->createObject(d->m_objectType);
    d->m_object->fromEnginioJson(
                QJsonDocument::fromJson(object.toEnginioJson()).object());
    d->m_objectOwned = true;
}

/*!
 * Set operation to read all \a object properties from the Enginio backend.
 * \a object must have \c objectType and \c id properties set. After a successful
 * operation, \a object is updated, so make sure it is not deleted before the
 * operation emits \c finished signal.
 *
 * If operation's \c model property is defined, corresponding object in the model
 * is also updated.
 */
void EnginioObjectOperation::read(EnginioAbstractObject *object)
{
    Q_D(EnginioObjectOperation);
    d->m_type = Enginio::ReadObjectOperation;
    d->m_objectId = object->id();
    d->m_objectType = object->objectType();
    d->m_object = object;
}

/*!
 * Set operation to read all \a object properties from the Enginio backend.
 * \a object must have \c objectType and \c id properties set.
 *
 * If operation's \c model property is defined, corresponding object in the
 * model is also updated.
 */
void EnginioObjectOperation::read(const EnginioAbstractObject &object)
{
    Q_D(EnginioObjectOperation);
    d->m_type = Enginio::ReadObjectOperation;
    d->m_objectId = object.id();
    d->m_objectType = object.objectType();
}

/*!
 * Create new object with \a id and \a objectType, and set operation to read rest
 * of the object properties from the Enginio backend. After a successful
 * operation, returned object is updated. Ensure that the object is not deleted
 * before the operation emits the \c finished signal. Returned object is owned
 * and deleted by the method caller.
 *
 * If operation's \c model property is defined, corresponding object in the
 * model is also updated.
 */
EnginioAbstractObject * EnginioObjectOperation::read(const QString &id,
                                                     const QString &objectType)
{
    Q_D(EnginioObjectOperation);
    d->m_type = Enginio::ReadObjectOperation;
    d->m_objectId = id;
    d->m_objectType = objectType;
    d->m_object = d->m_client->createObject(objectType, id);
    return d->m_object;
}

/*!
 * Set operation to update \a object properties in the Enginio backend.
 * In addition to changed properties, \a object must have \c objectType and
 * \c id properties set. After a successful operation, \a object is updated.
 * Ensure that the object is not deleted before the operation emits
 * \c finished signal.
 *
 * If operation's \c model property is defined, corresponding object in the
 * model is also updated.
 */
void EnginioObjectOperation::update(EnginioAbstractObject *object)
{
    qDebug() << "   ->" << Q_FUNC_INFO;
    Q_D(EnginioObjectOperation);
    d->m_type = Enginio::UpdateObjectOperation;
    d->m_objectId = object->id();
    d->m_objectType = object->objectType();
    d->m_object = object;
}

/*!
 * Set operation to update \a object properties in the Enginio backend.
 * In addition to changed properties, \a object must have \c objectType and
 * \c id properties set.
 *
 * If operation's \c model property is defined, corresponding object in the
 * model is also updated.
 */
void EnginioObjectOperation::update(const EnginioAbstractObject &object)
{
    Q_D(EnginioObjectOperation);
    d->m_type = Enginio::UpdateObjectOperation;
    d->m_objectId = object.id();
    d->m_objectType = object.objectType();
    d->m_object = d->m_client->createObject(d->m_objectType, d->m_objectId);
    d->m_object->fromEnginioJson(
                QJsonDocument::fromJson(object.toEnginioJson()).object());
    d->m_objectOwned = true;
}

/*!
 * Set operation to remove \a object from the Enginio backend. \a object must have
 * \c objectType and \c id properties set.
 *
 * If operation's \c model property is defined, corresponding object is also
 * removed from the model.
 */
void EnginioObjectOperation::remove(EnginioAbstractObject *object)
{
    Q_D(EnginioObjectOperation);
    d->m_type = Enginio::RemoveObjectOperation;
    d->m_objectId = object->id();
    d->m_objectType = object->objectType();
    d->m_object = object;
}

/*!
 * Set operation to remove \a object from the Enginio backend.
 * \a object must have \c objectType and \c id properties set.
 *
 * If operation's \c model property is defined, corresponding object is also
 * removed from the model.
 */
void EnginioObjectOperation::remove(const EnginioAbstractObject &object)
{
    Q_D(EnginioObjectOperation);
    d->m_type = Enginio::RemoveObjectOperation;
    d->m_objectId = object.id();
    d->m_objectType = object.objectType();
}

/*!
 * Set operation to remove object with \a id and \a objectType from the
 * Enginio backend.
 *
 * If operation's \c model property is defined, corresponding object is also
 * removed from the model.
 */
void EnginioObjectOperation::remove(const QString &id, const QString &objectType)
{
    Q_D(EnginioObjectOperation);
    d->m_type = Enginio::RemoveObjectOperation;
    d->m_objectId = id;
    d->m_objectType = objectType;
}
