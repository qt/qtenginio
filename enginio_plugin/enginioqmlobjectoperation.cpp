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

#include "enginioqmlobjectoperation.h"
#include "enginiojsonobject.h"
#include "enginioplugin.h"

#include <QDebug>
#include <QJsonDocument>
#include <QQmlEngine>

/*!
 * \qmltype ObjectOperation
 * \instantiates EnginioQmlObjectOperation
 * \inqmlmodule enginio-plugin
 * \brief Operation acting on a single Enginio object.
 *
 * Can be used to create new objects as well as read, update, and remove existing
 * objects from the Enginio backend.
 */

/*!
 * \qmlproperty Client ObjectOperation::client
 * Enginio client object. This property must be set before the operation is executed.
 */

/*!
 * \qmlproperty ObjectModel ObjectOperation::model
 * Object model that is updated when the operation finishes.
 */

/*!
 * \qmlproperty int ObjectOperation::modelIndex
 * Position at which new objects are added to ObjectOperation::model. By
 * default, new objects are added to the end of the model.
 */

/*!
 * \qmlsignal ObjectOperation::finished()
 * Emitted when the operation completes execution.
 */

/*!
 * \qmlsignal ObjectOperation::error(Error error)
 * Emitted when an error occurs while the operation is being executed. \a error contains
 * the error details.
 */

/*!
 * \qmlmethod void ObjectOperation::execute()
 * Execute operation asynchronously. When the operation finishes, \c finished signal
 * is emitted. If there's an error, both \c error and \c finished signals are
 * emitted.
 */

/*!
 * \qmlmethod void ObjectOperation::cancel()
 * Cancel ongoing operation. \c error signal will be emitted with
 * the \c networkError, \c QNetworkReply::OperationCanceledError.
 */

EnginioQmlObjectOperation::EnginioQmlObjectOperation(EnginioQmlClient *client,
                                                     EnginioQmlObjectModel *model,
                                                     QObject *parent) :
    EnginioObjectOperation(client, model, parent),
    m_object(0),
    m_modelIndexRow(-1)
{
    connect(this, SIGNAL(objectUpdated()), this, SLOT(updateObject()));
}

EnginioQmlClient * EnginioQmlObjectOperation::getClient() const
{
    return static_cast<EnginioQmlClient*>(client());
}

EnginioQmlObjectModel * EnginioQmlObjectOperation::getModel() const
{
    return static_cast<EnginioQmlObjectModel*>(model());
}

void EnginioQmlObjectOperation::setModelQml(EnginioQmlObjectModel *model)
{
    setModel(model);
    setModelIndexRow(m_modelIndexRow);
}

/*!
 * \qmlmethod void ObjectOperation::create(object object)
 *
 * Set operation to create \a object in the Enginio backend. \a object must have
 * \c objectType and any custom properties set. After a successful operation,
 * \c id property in \a object is updated.
 *
 * If operation's \c model property is defined, new object is added to
 * the model at position specified in \c modelIndex property.
 */
void EnginioQmlObjectOperation::create(QJSValue object)
{
    if (setObject(object))
        EnginioObjectOperation::create(m_object);
}

/*!
 * \qmlmethod void ObjectOperation::read(object object)
 *
 * Set operation to read all \a object properties from the Enginio backend.
 * \a object must have \c objectType and \c id properties set. After a successful
 * operation, \a object is updated.
 *
 * If operation's \c model property is defined, corresponding object in the model
 * is also updated.
 */
void EnginioQmlObjectOperation::read(QJSValue object)
{
    if (setObject(object))
        EnginioObjectOperation::read(m_object);
}

/*!
 * \qmlmethod void ObjectOperation::update(object object)
 *
 * Set operation to update \a object properties in the Enginio backend.
 * In addition to changed properties, \a object must have \c objectType and
 * \c id properties set. After a successful operation, \a object is updated.
 *
 * If operation's \c model property is defined, corresponding object in the
 * model is also updated.
 */
void EnginioQmlObjectOperation::update(QJSValue object)
{
    if (setObject(object))
        EnginioObjectOperation::update(m_object);
}

/*!
 * \qmlmethod void ObjectOperation::remove(object object)
 *
 * Set operation to remove \a object from the Enginio backend. \a object must have
 * \c objectType and \c id properties set.
 *
 * If operation's \c model property is defined, corresponding object is also
 * removed from the model.
 */
void EnginioQmlObjectOperation::remove(QJSValue object)
{
    if (setObject(object))
        EnginioObjectOperation::remove(m_object);
}

void EnginioQmlObjectOperation::updateObject()
{
    if (m_object)
        m_jsObject = g_qmlEngine->toScriptValue<QJsonObject>(
                    QJsonDocument::fromJson(m_object->toEnginioJson()).object());
}


bool EnginioQmlObjectOperation::setObject(const QJSValue &object)
{
    m_jsObject = object;

    if (m_object)
        delete m_object;

    if (!g_qmlEngine)
        return false;

    m_object = new EnginioJsonObject();
    if (!m_object)
        return false;

    m_object->fromEnginioJson(g_qmlEngine->fromScriptValue<QJsonObject>(object));
    return true;
}

int EnginioQmlObjectOperation::modelIndexRow() const
{
    if (model()) {
        QModelIndex index = modelIndex();
        if (index.isValid())
            return index.row();

        return -1;
    }
    return m_modelIndexRow;
}

void EnginioQmlObjectOperation::setModelIndexRow(int row)
{
    // In case index is set before model, keep it in m_modelIndexRow and set
    // when model is set.
    m_modelIndexRow = row;

    EnginioObjectModel *objectModel = model();
    if (objectModel) {
        QModelIndex index = objectModel->index(row);
        if (index.isValid())
            setModelIndex(index);
    }
}
