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

#include "enginioqueryoperation_p.h"
#include "enginioabstractobject.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

/*!
 * \class EnginioQueryOperation
 * \inmodule enginio-client
 * \brief Operation for fetching multiple Enginio objects from backend.
 *
 * Enginio Rest API provides two separate API endpoints for fetching objects
 * from backend. \l {https://engin.io/documentation/rest/endpoints/objects/query}
 * {Object queries} can be used to fetch objects of single type using MongoDB
 * style query parameter. Alternatively,
 * \l {https://engin.io/documentation/rest/endpoints/search/fulltext}
 * {fulltext search} can be used to search objects of multiple types where
 * object property contains a search string (defined in \c search parameter).
 *
 * EnginioQueryOperation selects correct API endpoint based on given object
 * types and request parameters. If more than one object type and \c search
 * parameter is defined, search API is used, otherwise object query API is used.
 *
 * Operation with more than one object type and no \c search parameter is
 * invalid will cause an error when executed.
 */

EnginioQueryOperationPrivate::EnginioQueryOperationPrivate(
        EnginioQueryOperation *op) :
    EnginioOperationPrivate(op),
    m_objectTypes(),
    m_model(0),
    m_modelIndex(),
    m_resultMetadata(),
    m_resultList()
{
}

EnginioQueryOperationPrivate::~EnginioQueryOperationPrivate()
{
    while (!m_resultList.isEmpty()) {
        delete m_resultList.takeFirst();
    }
}

QString EnginioQueryOperationPrivate::requestPath() const
{
    if (m_objectTypes.isEmpty())
        return QString();

    if (m_objectTypes.size() > 1 ||
       !m_requestParams.value(QStringLiteral("search")).isEmpty())
    {
        return QStringLiteral("/v1/search");
    }

    // If object type starts with "objects.", url should be "/v1/objects/type".
    // If not, url should be /v1/type.
    QString objectType(m_objectTypes.at(0));
    if (objectType.startsWith(QStringLiteral("objects.")))
        objectType[7] = QChar::fromLatin1('/');

    return QStringLiteral("/v1/") + objectType;
}

QNetworkReply * EnginioQueryOperationPrivate::doRequest(const QUrl &backendUrl)
{
    Q_Q(EnginioQueryOperation);

    QString path = requestPath();
    QString error;

    if (m_objectTypes.isEmpty())
        error = QStringLiteral("Unknown object type");
    else if (path.isEmpty())
        error = QStringLiteral("Request URL creation failed");
    else if (m_objectTypes.size() > 1 &&
             m_requestParams.value(QStringLiteral("search")).isEmpty()) {
        error = QStringLiteral("Missing search parameter");
    }
    if (!error.isEmpty()) {
        setError(EnginioError::RequestError, error);
        emit q->finished();
        return 0;
    }

    QUrl url(backendUrl);
    url.setPath(path);
    url.setQuery(urlQuery());

    if (m_objectTypes.size() > 1 ||
        !m_requestParams.value(QStringLiteral("search")).isEmpty())
    {
        QUrlQuery query(url.query());
        for (int i = 0; i < m_objectTypes.size(); i++) {
            query.addQueryItem(QStringLiteral("objectTypes[]"),
                               m_objectTypes.at(i));
        }
        url.setQuery(query);
    }

    QNetworkRequest req = enginioRequest(url);
    return m_client->networkManager()->get(req);
}

void EnginioQueryOperationPrivate::handleResults()
{
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
    if (json.contains(QStringLiteral("query")))
        m_resultMetadata = json.value(QStringLiteral("query")).toObject();
    if (!json.contains(QStringLiteral("results")))
        return;

    QJsonArray results = json.value(QStringLiteral("results")).toArray();
    qDebug() << "Number of objects fetched:" << results.count();

    // Create new objects
    QList<EnginioAbstractObject*> objects;
    for (int i = 0; i < results.size(); i++) {
        QJsonValue val = results.at(i);
        if (!val.isObject())
            continue;
        QJsonObject json = val.toObject();
        QString id = json.value(QStringLiteral("id")).toString();
        QString objectType = json.value(QStringLiteral("objectType")).toString();
        EnginioAbstractObject *obj = m_client->createObject(objectType, id);
        if (obj) {
            obj->fromEnginioJson(json);
            if (!obj->fromEnginioJson(json)) {
                delete obj;
                setError(EnginioError::RequestError,
                         QStringLiteral("Invalid reply object"));
             } else {
                objects << obj;
            }
        }
    }

    // Add new objects to model or resultList
    if (objects.size() > 0) {

        if (m_model) {
            int row = -1;
            if (m_modelIndex.isValid())
                row = m_modelIndex.row();
            m_model->addToModel(objects, row);
        } else {
            m_resultList.append(objects);
        }
    }
}


/*!
 * Create new query operation. \a client must be valid pointer to EnginioClient
 * object. If \a model is specified, query result objects will be added to the
 * model. \a parent is optional parent QObject.
 */
EnginioQueryOperation::EnginioQueryOperation(EnginioClient *client,
                                             EnginioObjectModel *model,
                                             QObject *parent) :
    EnginioOperation(client, *new EnginioQueryOperationPrivate(this), parent)
{
    qDebug() << this << "created";
    if (model)
        setModel(model);
}

/*!
 * Constructor used in inheriting classes.
 */
EnginioQueryOperation::EnginioQueryOperation(EnginioClient *client,
                                             EnginioObjectModel *model,
                                             EnginioQueryOperationPrivate &dd,
                                             QObject *parent) :
    EnginioOperation(client, dd, parent)
{
    qDebug() << this << "created";
    if (model)
        setModel(model);
}

/*!
 * Destructor.
 */
EnginioQueryOperation::~EnginioQueryOperation()
{
    qDebug() << this << "deleted";
}

/*!
 * \obsolete
 * Use objectTypes() instead.
 *
 * Get object type defined for this operation.
 */
QString EnginioQueryOperation::objectType() const
{
    Q_D(const EnginioQueryOperation);
    return d->m_objectTypes.at(0);
}

/*!
 * \obsolete
 * Use setObjectTypes() or addObjectType() instead.
 *
 * Define what types of objects should be queried. Query will search and return
 * only objects of type \a objectType.
 */
void EnginioQueryOperation::setObjectType(const QString &objectType)
{
    Q_D(EnginioQueryOperation);
    d->m_objectTypes.clear();
    d->m_objectTypes.append(objectType);
}

/*!
 * Get the model set for this operation. If model has not been defined, returned
 * pointer is 0.
 */
EnginioObjectModel * EnginioQueryOperation::model() const
{
    Q_D(const EnginioQueryOperation);
    return d->m_model;
}

/*!
 * Define model where result objects are added when operation finishes. Make
 * sure \a model is not deleted before operation has finished.
 */
void EnginioQueryOperation::setModel(EnginioObjectModel *model)
{
    Q_D(EnginioQueryOperation);
    d->m_model = model;
}

/*!
 * Get model index where result objects are inserted in model.
 */
QModelIndex EnginioQueryOperation::modelIndex() const
{
    Q_D(const EnginioQueryOperation);
    return d->m_modelIndex;
}

/*!
 * Set model \a index where result objects are inserted in model. By default
 * objects are added to the end of the model.
 */
void EnginioQueryOperation::setModelIndex(QModelIndex index)
{
    Q_D(EnginioQueryOperation);
    d->m_modelIndex = index;
}

/*!
 * Returns query results if \c model parameter is not specified. Calling this
 * method will remove all received result objects from internal storage and
 * return them in a list. It is caller's responsibility to delete objects in the
 * list.
 */
QList<EnginioAbstractObject*> EnginioQueryOperation::takeResults()
{
    Q_D(EnginioQueryOperation);
    QList<EnginioAbstractObject*> result(d->m_resultList);
    d->m_resultList.clear();
    return result;
}

/*!
 * List of object types to be queried.
 */
QStringList EnginioQueryOperation::objectTypes() const
{
    Q_D(const EnginioQueryOperation);
    return d->m_objectTypes;
}

/*!
 * Define what types of objects should be queried. All previously defined object
 * types will be overwritten by \a objectTypes. At least one object type must be
 * defined before operation is executed.
 */
void EnginioQueryOperation::setObjectTypes(const QStringList &objectTypes)
{
    Q_D(EnginioQueryOperation);
    d->m_objectTypes = objectTypes;
}

/*!
 * Add \a objectType to list of object types to be queried. At least one object
 * type must be defined before operation is executed.
 */
void EnginioQueryOperation::addObjectType(const QString &objectType)
{
    Q_D(EnginioQueryOperation);
    if (!d->m_objectTypes.contains(objectType))
        d->m_objectTypes.append(objectType);
}
