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

#include "enginioqmlusergroupoperation.h"
#include "enginiojsonobject.h"
#include "enginioplugin.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJSValueIterator>
#include <QQmlEngine>

/*!
 * \qmltype UsergroupOperation
 * \instantiates EnginioQmlUsergroupOperation
 * \inqmlmodule enginio-plugin
 * \brief Operation for adding and removing usergroup members.
 *
 * Use this operation to add or remove users to/from usergroup. For
 * getting members of a usergroup use \l QueryOperation (specifically
 * \l QueryOperation::usergroupId property). New usergroups (like any
 * other Enginio objects) can be created with \l ObjectOperation.
 *
 * Similar to \l ObjectOperation \c UsergroupOperation supports
 * automatic updating of object models. E.g. usergroup members can be fetched to
 * \l ObjectModel by setting usergroup ID and model to query operation.
 * Setting same model to \c UsergroupOperation will add user object to
 * the model when user is added to usergroup and delete user object from the
 * model when member is removed from usergroup.
 *
 * This operation also supports
 * \l {https://engin.io/documentation/rest/parameters/include}
 * {\c include request parameter}. It can be used to specify what data is
 * included in the user objects which are added to object model.
 */

/*!
 * \qmlproperty Client UsergroupOperation::client
 * Enginio client object. This property must be set before the operation is
 * executed.
 */

/*!
 * \qmlproperty ObjectModel UsergroupOperation::model
 * Object model that is updated when the operation finishes.
 */

/*!
 * \qmlproperty int UsergroupOperation::modelIndex
 * Position at which new objects are added to ObjectOperation::model. By
 * default, new objects are added to the end of the model.
 */

/*!
 * \qmlproperty object UsergroupOperation::user
 * The user object given as argument to addMember or removeMember function.
 */

/*!
 * \qmlproperty object UsergroupOperation::include
 * Parameter for defining what data should be included in the user objects
 * added to \l model after \l addMember(). For more info see
 * \l {https://engin.io/documentation/rest/parameters/include}
 * {include parameter documentation.}
 */

/*!
 * \qmlsignal UsergroupOperation::finished()
 * Emitted when the operation completes execution.
 */

/*!
 * \qmlsignal UsergroupOperation::error(Error error)
 * Emitted when an error occurs while the operation is being executed. \a error
 * contains the error details.
 */

/*!
 * \qmlmethod void UsergroupOperation::execute()
 * Execute operation asynchronously. When the operation finishes, \c finished
 * signal is emitted. If there's an error, both \c error and \c finished signals
 * are emitted (in that order).
 */

/*!
 * \qmlmethod void UsergroupOperation::cancel()
 * Cancel ongoing operation. \c error signal will be emitted with
 * the \c networkError, \c QNetworkReply::OperationCanceledError.
 */

/*!
 * \qmlmethod string UsergroupOperation::requestParam(string name)
 * Get request parameter with \a name. If request parameter with \a name has not
 * been set, returns empty string.
 */

/*!
 * \qmlmethod void UsergroupOperation::setRequestParam(string name, string value)
 * Set request parameter with \a name and \a value to be added to request URL. If
 * request parameter with same \a name has already been set, the old value will
 * be overwritten. Setting parameter with empty \a value will remove already set
 * parameter.
 *
 * Refer to the Enginio REST API documentation for valid parameters and value
 * syntax.
 */

EnginioQmlUsergroupOperation::EnginioQmlUsergroupOperation(EnginioQmlClient *client,
                                                           EnginioQmlObjectModel *model,
                                                           QObject *parent) :
    EnginioUsergroupOperation(client, model, parent),
    m_user(0),
    m_modelIndexRow(-1)
{
    connect(this, SIGNAL(userUpdated()), this, SLOT(updateUser()));
}

EnginioQmlClient * EnginioQmlUsergroupOperation::getClient() const
{
    return static_cast<EnginioQmlClient*>(client());
}

EnginioQmlObjectModel * EnginioQmlUsergroupOperation::getModel() const
{
    return static_cast<EnginioQmlObjectModel*>(model());
}

void EnginioQmlUsergroupOperation::setModelQml(EnginioQmlObjectModel *model)
{
    setModel(model);
    setModelIndexRow(m_modelIndexRow);
}

QJSValue EnginioQmlUsergroupOperation::user()
{
    return m_jsObject;
}

QJsonObject EnginioQmlUsergroupOperation::include() const
{
    QByteArray includeString = requestParam(QStringLiteral("include")).toUtf8();
    return QJsonDocument::fromJson(includeString).object();
}

void EnginioQmlUsergroupOperation::setInclude(const QJsonObject &include)
{
    setRequestParam(QStringLiteral("include"),
                    QString(QJsonDocument(include).toJson()).simplified());
}

/*!
 * \qmlmethod void UsergroupOperation::addMember(object user, string usergroupId)
 *
 * Set operation to add \a user to usergroup with \a usergroupId. \a user must
 * have valid \c id property. All other properties will be ignored. After
 * a successful operation, \a user is updated to contain full user object.
 *
 * If operation's \c model property is defined, updated \a user object is added
 * to the model at position specified in \c modelIndex property.
 */
void EnginioQmlUsergroupOperation::addMember(QJSValue user, const QString &usergroupId)
{
    if (setUser(user))
        EnginioUsergroupOperation::addMember(m_user, usergroupId);
}

/*!
 * \qmlmethod void UsergroupOperation::addMemberById(string userId, string usergroupId)
 *
 * Set operation to add user with \a userId to usergroup with \a usergroupId.
 *
 * If operation's \c model property is defined, user object with \a userId is
 * added to the model at position specified in \c modelIndex property.
 */
void EnginioQmlUsergroupOperation::addMemberById(const QString &userId,
                                                 const QString &usergroupId)
{
    EnginioUsergroupOperation::addMember(userId, usergroupId);
}

/*!
 * \qmlmethod void UsergroupOperation::removeMember(object user, string usergroupId)
 *
 * Set operation to remove \a user from usergroup with \a usergroupId. \a user
 * must have valid \c id property.
 *
 * If operation's \c model property is defined, corresponding user object is
 * also removed from the model.
 */
void EnginioQmlUsergroupOperation::removeMember(QJSValue user, const QString &usergroupId)
{
    if (setUser(user))
        EnginioUsergroupOperation::removeMember(m_user, usergroupId);
}

/*!
 * \qmlmethod void UsergroupOperation::removeMemberById(string userId, string usergroupId)
 *
 * Set operation to remove user with \a userId from usergroup with
 * \a usergroupId.
 *
 * If operation's \c model property is defined, corresponding user object is
 * also removed from the model.
 */
void EnginioQmlUsergroupOperation::removeMemberById(const QString &userId,
                                                    const QString &usergroupId)
{
    EnginioUsergroupOperation::removeMember(userId, usergroupId);
}

void EnginioQmlUsergroupOperation::updateUser()
{
    if (m_user) {
        QJSValue object = g_qmlEngine->toScriptValue<QJsonObject>(
                    QJsonDocument::fromJson(m_user->toEnginioJson()).object());
        QJSValueIterator iter(object);
        while (iter.hasNext()) {
            iter.next();
            m_jsObject.setProperty(iter.name(), iter.value());
        }
    }
}

bool EnginioQmlUsergroupOperation::setUser(const QJSValue &user)
{
    m_jsObject = user;

    if (m_user)
        delete m_user;

    if (!g_qmlEngine)
        return false;

    m_user = new EnginioJsonObject();
    if (!m_user)
        return false;

    m_user->fromEnginioJson(g_qmlEngine->fromScriptValue<QJsonObject>(user));
    return true;
}

int EnginioQmlUsergroupOperation::modelIndexRow() const
{
    if (model()) {
        QModelIndex index = modelIndex();
        if (index.isValid())
            return index.row();

        return -1;
    }
    return m_modelIndexRow;
}

void EnginioQmlUsergroupOperation::setModelIndexRow(int row)
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
