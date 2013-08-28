/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtEnginio module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
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
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "enginioqmlreply.h"
#include "enginioqmlclient_p.h"
#include "enginioqmlclient.h"
#include <Enginio/private/enginioreply_p.h>
#include <QtQml/qjsvalue.h>
#include <QtQml/qqmlengine.h>
#include <QtCore/qobject.h>
#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

/*!
  \qmltype EnginioReply
  \instantiates EnginioQmlReply
  \inqmlmodule Enginio 1
  \ingroup engino-qml
  \target EnginioModelQml

  \brief A reply to any \l Enginio request.
*/

/*!
  \qmlproperty QJsonObject Enginio1::EnginioReply::data
  The data of this reply.
*/

/*!
  \qmlproperty EnginioReply::ErrorTypes Enginio1::EnginioReply::errorType
  The type of error (if any) of this reply.
*/

/*!
  \qmlproperty QNetworkReply::NetworkError Enginio1::EnginioReply::networkError
  The network error (if any).
*/

/*!
  \qmlproperty string Enginio1::EnginioReply::errorString
  The error message if this reply was an error.
*/

/*!
  \qmlproperty int Enginio1::EnginioReply::backendStatus
  The backend status code.
*/

class EnginioQmlReplyPrivate : public EnginioReplyPrivate
{
    EnginioQmlReply *q;
    QMetaObject::Connection _orphanConnection;
    mutable QJSValue _value;

public:
    EnginioQmlReplyPrivate(EnginioQmlClientPrivate *client, QNetworkReply *reply, EnginioQmlReply *q_ptr)
        : EnginioReplyPrivate(client, reply)
        , q(q_ptr)
    {
        Q_ASSERT(client);
    }

    void emitFinished()
    {
        // Enginio doesn't need this reply anymore, so now we need to figure out how to
        // delete it. There are two cases:
        //  - finished and error signals are not connected => nobody really cares about
        //    this reply we can safely delete it.
        //  - at least one of finished and error signals is connected => we assume that
        //    the connection is done from QML, so we transfer reply ownership to it. C++
        //    developers needs to take care of such situation and addapt.

        static QMetaMethod clientFinishedSignal = QMetaMethod::fromSignal(&EnginioQmlClient::finished);
        static QMetaMethod clientErrorSignal = QMetaMethod::fromSignal(&EnginioQmlClient::error);
        static QMetaMethod replyFinishedSignal = QMetaMethod::fromSignal(&EnginioQmlReply::finished);

        // TODO it will not work because of: https://bugreports.qt-project.org/browse/QTBUG-32340
        bool isReplyFinishedConnected = q->isSignalConnected(replyFinishedSignal);
        bool isClientFinishedConnected = _client->isSignalConnected(clientFinishedSignal);
        bool isClientErrorConnected = _client->isSignalConnected(clientErrorSignal);

        if (Q_LIKELY(isClientFinishedConnected
                  || isReplyFinishedConnected
                  || isClientErrorConnected)) {
            // something is connected and we can transfer the owership.
            q->setParent(0);
            QQmlEngine::setObjectOwnership(q, QQmlEngine::JavaScriptOwnership);
            if (isReplyFinishedConnected)
                emit q->finished(q);
        } else
            q->deleteLater();
    }

    QJSValue data() const
    {
        if (!_value.isObject()) {
            if (_data.isEmpty()) {
                QByteArray replyData = _nreply->readAll();
                _data = QJsonDocument::fromJson(replyData).object();
                _value = static_cast<EnginioQmlClientPrivate*>(_client)->fromJson(replyData);
            } else {
                _value = static_cast<EnginioQmlClientPrivate*>(_client)->fromJson(
                            QJsonDocument(_data).toJson(QJsonDocument::Compact));
            }
        }
        return _value;
    }
};


EnginioQmlReply::EnginioQmlReply(EnginioQmlClientPrivate *parent, QNetworkReply *reply)
    : EnginioReply(parent, reply, new EnginioQmlReplyPrivate(parent, reply, this))
{}

EnginioQmlReply::~EnginioQmlReply()
{
    EnginioQmlReplyPrivate *priv = static_cast<EnginioQmlReplyPrivate *>(d.take());
    delete priv;
}

QJSValue EnginioQmlReply::data() const
{
    return static_cast<const EnginioQmlReplyPrivate*>(d.data())->data();
}

/*!
  \internal
*/
void EnginioQmlReply::emitFinished()
{
    static_cast<EnginioQmlReplyPrivate*>(d.data())->emitFinished();
}

QT_END_NAMESPACE
