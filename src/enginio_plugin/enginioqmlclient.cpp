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

#include "enginioqmlacloperation.h"
#include "enginioqmlclient.h"
#include "Enginio/private/enginioclient_p.h"
#include "enginioqmlidentityauthoperation.h"
#include "enginioqmlobjectoperation.h"
#include "enginioqmlqueryoperation.h"
#include "enginioqmlobjectadaptor_p.h"
#include "enginioqmlusergroupoperation.h"

#include <QtQml/qjsvalue.h>

#include <QDebug>

/*!
 * \qmltype Enginio
 * \instantiates EnginioQmlClient
 * \inqmlmodule enginio-plugin
 * \brief Enginio client inteface to access the service.
 *
 * Enginio is used for all communication with the Enginio backend.
 * The backend is identified by \l{backendId}{backend ID} and \l{backendSecret}{secret}.
 *
 * It also provides methods for creating operation objects dynamically.
 */

/*!
 * \qmlproperty string Enginio::backendId
 * Enginio backend ID. This can be obtained from the Enginio dashboard.
 */

/*!
 * \qmlproperty string Enginio::backendSecret
 * Enginio backend secret. This can be obtained from the Enginio dashboard.
 */

/*!
 * \qmlproperty string Enginio::apiUrl
 * Enginio backend URL.
 *
 * Usually there is no need to change the default URL.
 */

/*!
 * \qmlproperty string Enginio::sessionToken
 * Token of currently authenticated session.
 *
 * The session token is an empty string if there is no
 * authenticated session.
 */

/*!
 * \qmlproperty bool Enginio::isAuthenticated()
 * This property holds the state of the authentication.
 *
 * It is false until a user session has been establieshed.
 * \sa sessionToken
 */

class IsAuthenticatedFunctor
{
    EnginioQmlClient *_qmlEnginio;
    bool _status;

public:
    IsAuthenticatedFunctor(EnginioQmlClient *qmlEnginio, bool status)
        : _qmlEnginio(qmlEnginio)
        , _status(status)
    {}
    void operator ()()
    {
        _qmlEnginio->isAuthenticatedChanged(_status);
    }
};

EnginioQmlClient::EnginioQmlClient(QObject *parent)
    : EnginioClient(parent, new EnginioQmlClientPrivate(this))
{
    qRegisterMetaType<EnginioQmlClient*>();
    qRegisterMetaType<EnginioQmlReply*>();

    QObject::connect(this, &EnginioClient::sessionAuthenticated, IsAuthenticatedFunctor(this, true));
    QObject::connect(this, &EnginioClient::sessionTerminated, IsAuthenticatedFunctor(this, false));
}

/*!
  \internal
 * \qmlmethod Enginio::createObjectOperation(ObjectModel model = 0)
 * Returns new \l ObjectOperation which can be used to create new objects on
 * backend or read, update or delete existing objects. If \a model is specified,
 * when operation finishes corresponding object in \a model will be updated.
 * Returned operation can be deleted with \c destroy().
 */
EnginioQmlObjectOperation * EnginioQmlClient::createObjectOperation(
        EnginioQmlObjectModel *model)
{
    return new EnginioQmlObjectOperation(this, model);
}

/*!
  \internal
 * \qmlmethod Enginio::createQueryOperation(ObjectModel model = 0)
 *
 * Returns new \l QueryOperation which can be used to query objects from
 * backend. If \a model is specified, when operation finishes any objects
 * fetched from backend will be added to \a model. Returned operation can be
 * deleted with \c destroy().
 */
EnginioQmlQueryOperation * EnginioQmlClient::createQueryOperation(
        EnginioQmlObjectModel *model)
{
    return new EnginioQmlQueryOperation(this, model);
}

/*!
  \internal
 * \qmlmethod Enginio::createIdentityAuthOperation()
 *
 * Returns new \l IdentityAuthOperation which can be used to authenticate user
 * with Enginio backend. Returned operation can be deleted with \c destroy().
 */
EnginioQmlIdentityAuthOperation * EnginioQmlClient::createIdentityAuthOperation()
{
    return new EnginioQmlIdentityAuthOperation(this);
}

/*!
  \internal
 * \qmlmethod Enginio::createAclOperation()
 *
 * Returns new \l AclOperation which can be used to read and modify permissions
 * of Enginio objects. Returned operation can be deleted with \c destroy().
 */
EnginioQmlAclOperation * EnginioQmlClient::createAclOperation()
{
    return new EnginioQmlAclOperation(this);
}

bool EnginioQmlClient::isAuthenticated() const
{
    return sessionToken().isEmpty();
}

EnginioQmlReply *EnginioQmlClient::query(const QJSValue &query, const Operation operation)
{
    Q_D(EnginioQmlClient);

    d->setEngine(query);
    ObjectAdaptor<QJSValue> o(query, d);
    QNetworkReply *nreply = d_ptr->query<QJSValue>(o, static_cast<EnginioClientPrivate::Operation>(operation));
    EnginioQmlReply *ereply = new EnginioQmlReply(d, nreply);
    nreply->setParent(ereply);
    return ereply;
}

EnginioQmlReply *EnginioQmlClient::create(const QJSValue &object, const Operation operation)
{
    Q_D(EnginioQmlClient);

    if (!object.isObject())
        return 0;

    d->setEngine(object);
    ObjectAdaptor<QJSValue> o(object, d);
    QNetworkReply *nreply = d_ptr->create<QJSValue>(o, operation);
    EnginioQmlReply *ereply = new EnginioQmlReply(d, nreply);
    nreply->setParent(ereply);

    return ereply;
}

EnginioQmlReply *EnginioQmlClient::update(const QJSValue &object, const Operation operation)
{
    Q_D(EnginioQmlClient);

    if (!object.isObject())
        return 0;

    d->setEngine(object);
    ObjectAdaptor<QJSValue> o(object, d);
    QNetworkReply *nreply = d_ptr->update<QJSValue>(o, operation);
    EnginioQmlReply *ereply = new EnginioQmlReply(d, nreply);
    nreply->setParent(ereply);

    return ereply;
}

EnginioQmlReply *EnginioQmlClient::remove(const QJSValue &object, const Operation operation)
{
    Q_D(EnginioQmlClient);

    if (!object.isObject())
        return 0;

    d->setEngine(object);
    ObjectAdaptor<QJSValue> o(object, d);
    QNetworkReply *nreply = d_ptr->remove<QJSValue>(o, operation);
    EnginioQmlReply *ereply = new EnginioQmlReply(d, nreply);
    nreply->setParent(ereply);

    return ereply;
}

QByteArray EnginioQmlClientPrivate::toJson(const QJSValue &value)
{
    if (!_stringify.isCallable())
        Q_UNIMPLEMENTED(); // TODO maybe _value.toString().toUtf8()?
    return _stringify.call(QJSValueList() << value).toString().toUtf8();
}

QJSValue EnginioQmlClientPrivate::fromJson(const QByteArray &value)
{
    if (!_parse.isCallable())
        Q_UNIMPLEMENTED();
    return _parse.call(QJSValueList() << _engine->toScriptValue(value));
}

void EnginioQmlClientPrivate::_setEngine(QJSEngine *engine)
{
    Q_ASSERT(!_engine);
    if (engine) {
        _engine = engine;
        _stringify = engine->evaluate("JSON.stringify");
        _parse = engine->evaluate("JSON.parse");
        Q_ASSERT(_stringify.isCallable());
    }
}

/*!
  \internal
 * \qmlmethod Enginio::createUsergroupOperation()
 *
 * Returns new \l UsergroupOperation which can be used to add and remove members
 * to/from usergroups. Returned operation can be deleted with \c destroy().
 */
EnginioQmlUsergroupOperation * EnginioQmlClient::createUsergroupOperation()
{
    return new EnginioQmlUsergroupOperation(this);
}
