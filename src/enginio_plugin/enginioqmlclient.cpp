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

#include "enginioqmlclient.h"
#include "Enginio/private/enginioclient_p.h"
#include "enginioqmlobjectadaptor_p.h"

#include <QtQml/qjsvalue.h>

#include <QDebug>

/*!
  \qmltype Enginio
  \instantiates EnginioQmlClient
  \inqmlmodule enginio-plugin
  \ingroup engino-qml

  \brief client interface to access Enginio

  \snippet simple.qml import

  Enginio is the heart of the QML API for Enginio.
  It is used for all communication with the Enginio backend.
  \l EnginioModel compliments it to make handling of multiple objects simple.

  The backend is identified by \l{backendId}{backend ID} and \l{backendSecret}{secret}.
  \snippet simple.qml client

  Once the backend is configured, it is possible to run queries by calling query on
  Enginio.
  For example to get all objects stored with the type "objects.image" run this query:
  \snippet simple.qml client-query

  Enginio gives you a convenient way to handle the responses to your queryies as well:
  \snippet simple.qml client-signals
*/

/*!
  \qmlproperty string Enginio::backendId
  Enginio backend ID. This can be obtained from the Enginio dashboard.
*/

/*!
  \qmlproperty string Enginio::backendSecret
  Enginio backend secret. This can be obtained from the Enginio dashboard.
*/

/*!
  \qmlproperty string Enginio::apiUrl
  Enginio backend URL.

  Usually there is no need to change the default URL.
*/

/*!
  \qmlproperty object Enginio::identityToken
  Token of currently authenticated session. It contains information about
  logged in user.

  The token is an empty object if there is no
  authenticated session.
*/

/*!
  \qmlproperty bool Enginio::isAuthenticated()
  This property holds the state of the authentication.

  It is false until a user session has been establieshed.
  \sa identityToken
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

bool EnginioQmlClient::isAuthenticated() const
{
    return !identityToken().isEmpty();
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

