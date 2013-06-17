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

#include <QtCore/qstring.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsondocument.h>
#include <QtNetwork/qnetworkreply.h>

#include "enginioreply.h"
#include "enginioreply_p.h"
#include "enginioclient.h"
#include "enginioclient_p.h"
#include "enginioobjectadaptor_p.h"

/*!
  \class EnginioReply
  \brief The EnginioReply class contains the data from a request to the Enginio database.
  \inmodule enginio-qt
  \ingroup enginio-client

  The finished signal is emitted when the query is done.

  \sa EnginioClient
*/

/*!
  \internal
*/
EnginioReply::EnginioReply(EnginioClientPrivate *p, QNetworkReply *reply)
    : QObject(p->q_ptr)
    , d(new EnginioReplyPrivate(p, reply))
{
    p->registerReply(reply, this);
}

/*!
  \internal
*/
EnginioReply::EnginioReply(EnginioClientPrivate *parent, QNetworkReply *reply, EnginioReplyPrivate *priv)
    : QObject(parent->q_ptr)
    , d(priv)
{
    parent->registerReply(reply, this);
}


/*!
  \brief Destroys the EnginioReply.

  The reply needs to be deleted after the finished signal is emitted.
*/
EnginioReply::~EnginioReply()
{
    // At this point d pointer may be null, because it may be deleted by
    // EnginioQmlReply. If you need to add code here, then consider to
    // make EnginioReply destructor virtual. For now there is no point
    // in adding vtable to the private class.
}

/*!
 * \brief EnginioReply::errorCode gives the network error for the request.
 */
QNetworkReply::NetworkError EnginioReply::networkError() const
{
    return d->errorCode();
}

/*!
 * \brief EnginioReply::errorCode gives the network error for the request as human readable string.
 */
QString EnginioReply::errorString() const
{
    return d->errorString();
}

/*!
 * \brief EnginioReply::data returns the data of any request to the Enginio backend.
 */
QJsonObject EnginioReply::data() const
{
    return d->data();
}

void EnginioReply::emitFinished()
{
    emit finished(this);
}

void EnginioReply::setNetworkReply(QNetworkReply *reply)
{
    d->_client->_replyReplyMap.remove(d->_nreply);

    if (gEnableEnginioDebugInfo)
        d->_client->_requestData.remove(d->_nreply);

    delete d->_nreply;
    d->_nreply = reply;
    d->_data = QJsonObject();
    d->_client->registerReply(reply, this);
}

void EnginioReply::dumpDebugInfo() const
{
    d->dumpDebugInfo();
}

int EnginioReply::backendStatus() const
{
    return d->backendStatus();
}

EnginioReply::ErrorTypes EnginioReply::errorType() const
{
    return d->errorType();
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const EnginioReply *reply)
{
    if (!reply) {
        d << "EnginioReply(null)";
        return d;
    }
    d.nospace();
    d << "EnginioReply(" << hex << (void *) reply << dec;

    if (reply->networkError() == 0) {
        d << " success data=" << reply->data();
    } else {
        d << " errorCode=" << reply->networkError() << " ";
        d << " errorString=" << reply->errorString() << " ";
        d << " errorData=" << reply->data() << " ";
    }
    d << ")";
    return d.space();
}
#endif // QT_NO_DEBUG_STREAM
