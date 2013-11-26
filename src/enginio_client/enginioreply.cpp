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

#include <QtCore/qstring.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsondocument.h>
#include <QtNetwork/qnetworkreply.h>

#include <Enginio/enginioreplybase.h>
#include <Enginio/enginioreply.h>
#include <Enginio/private/enginioreply_p.h>
#include <Enginio/enginioclient.h>
#include <Enginio/private/enginioclient_p.h>
#include <Enginio/private/enginioobjectadaptor_p.h>

QT_BEGIN_NAMESPACE

/*!
  \class EnginioReply
  \brief The EnginioReply class contains the data from a request to the Enginio database.
  \inmodule enginio-qt
  \ingroup enginio-client

  The finished signal is emitted when the query is done.

  \sa EnginioClient
*/

/*!
  \class EnginioReplyBase
  \internal
*/

/*!
  \enum EnginioReply::ErrorTypes
  Describes the type of error that occured when making a request to the Enginio backend.
  \value NoError The reply returned without errors
  \value NetworkError The error was a networking problem
  \value BackendError The backend did not accept the query
*/

/*!
  \fn EnginioReply::finished(EnginioReply *reply)
  This signal is emitted when the EnginioReply \a reply is finished.
  After the network operation, use the \l{EnginioReply::isError()}{isError()} function to check for
  potential problems and then use the \l data property to access the returned data.
*/

/*!
  \fn EnginioReply::progress(qint64 bytesSent, qint64 bytesTotal)
  This signal is emitted for file operations, indicating the progress of up or downloads.
  The \a bytesSent is the current progress relative to the total \a bytesTotal.
*/

class EnginioReplyPrivate: public EnginioReplyBasePrivate {
public:
    EnginioReplyPrivate(EnginioClientConnectionPrivate *p, QNetworkReply *reply)
        : EnginioReplyBasePrivate(p, reply)
    {}
};

/*!
  \internal
*/
EnginioReply::EnginioReply(EnginioClientConnectionPrivate *p, QNetworkReply *reply)
    : EnginioReplyBase(p, reply, new EnginioReplyPrivate(p, reply))
{
    QObject::connect(this, &EnginioReply::dataChanged, this, &EnginioReplyBase::dataChanged);
}

/*!
  \brief Destroys the EnginioReply.

  The reply needs to be deleted after the finished signal is emitted.
*/
EnginioReply::~EnginioReply()
{}

/*!
  \property EnginioReply::networkError
  This property holds the network error for the request.
*/

QNetworkReply::NetworkError EnginioReplyBase::networkError() const
{
    Q_D(const EnginioReplyBase);
    return d->errorCode();
}

/*!
  \property EnginioReply::errorString
  This property holds the error for the request as human readable string.
  Check \l{EnginioReply::isError()}{isError()} first to check if the reply is an error.
*/

QString EnginioReplyBase::errorString() const
{
    Q_D(const EnginioReplyBase);
    return d->errorString();
}

/*!
  \property EnginioReply::requestId
  This property holds the API request ID for the request.
  The request ID is useful for end-to-end tracking of requests and to identify
  the origin of notifications.
  \internal
*/

/*!
  \internal
*/
QString EnginioReplyBase::requestId() const
{
    Q_D(const EnginioReplyBase);
    return d->requestId();
}

/*!
  \property EnginioReply::data
  \brief The data returned from the backend
  This property holds the JSON data returned by the server after a successful request.
*/

QJsonObject EnginioReply::data() const
{
    Q_D(const EnginioReply);
    return d->data();
}

/*!
  \internal
*/
void EnginioReply::emitFinished()
{
    emit finished(this);
}

/*!
  \internal
*/
void EnginioReplyBase::setNetworkReply(QNetworkReply *reply)
{
    Q_D(EnginioReplyBase);
    d->setNetworkReply(reply);
}

void EnginioReplyBasePrivate::setNetworkReply(QNetworkReply *reply)
{
    Q_Q(EnginioReplyBase);
    _client->unregisterReply(_nreply);

    if (gEnableEnginioDebugInfo)
        _client->_requestData.remove(_nreply);

    if (!_nreply->isFinished()) {
        _nreply->setParent(_nreply->manager());
        QObject::connect(_nreply, &QNetworkReply::finished, _nreply, &QNetworkReply::deleteLater);
        _nreply->abort();
    } else {
        _nreply->deleteLater();
    }
    _nreply = reply;
    _data = QByteArray();

    _client->registerReply(reply, q);
}

/*!
  \internal
*/
void EnginioReplyBase::swapNetworkReply(EnginioReplyBase *other)
{
    Q_D(EnginioReplyBase);
    d->swapNetworkReply(other->d_func());
}

void EnginioReplyBasePrivate::swapNetworkReply(EnginioReplyBasePrivate *other)
{
    Q_Q(EnginioReplyBase);
    Q_ASSERT(other->_client == _client);
    _client->unregisterReply(_nreply);
    _client->unregisterReply(other->_nreply);

    qSwap(_nreply, other->_nreply);
    _data = other->_data = QByteArray();

    _client->registerReply(_nreply, q);
    _client->registerReply(other->_nreply, other->q_func());
}

/*!
  \internal
*/
void EnginioReplyBase::dumpDebugInfo() const
{
    Q_D(const EnginioReplyBase);
    d->dumpDebugInfo();
}

/*!
  \internal
  Mark this EnginioReply as not finished, the finished signal
  will be delayed until delayFinishedSignal() is returning true.

  \note The feature can be used only with one EnginioClient
*/
void EnginioReplyBase::setDelayFinishedSignal(bool delay)
{
    Q_D(EnginioReplyBase);
    d->_delay = delay;
    d->_client->finishDelayedReplies();
}

/*!
  \internal
  Returns true if signal should be delayed
 */
bool EnginioReplyBase::delayFinishedSignal()
{
    Q_D(EnginioReplyBase);
    return d->_delay;
}

/*!
  \fn bool EnginioReply::isError() const
  \brief EnginioReplyBase::isError returns whether this reply was unsuccessful
  \return true if the reply did not succeed
*/

bool EnginioReplyBase::isError() const
{
    Q_D(const EnginioReplyBase);
    return d->errorCode() != QNetworkReply::NoError;
}

/*!
  \fn bool EnginioReply::isFinished() const
  \brief Returns whether this reply was finished or not
  \return true if the reply was finished, false otherwise.
*/

bool EnginioReplyBase::isFinished() const
{
    Q_D(const EnginioReplyBase);
    return d->isFinished();
}

/*!
  \property EnginioReply::backendStatus
  \return the backend return status for this reply.
  \sa EnginioReplyBase::ErrorTypes
*/

int EnginioReplyBase::backendStatus() const
{
    Q_D(const EnginioReplyBase);
    return d->backendStatus();
}

/*!
  \property EnginioReply::errorType
  \return the type of the error
  \sa EnginioReplyBase::ErrorTypes
*/

EnginioReplyBase::ErrorTypes EnginioReplyBase::errorType() const
{
    Q_D(const EnginioReplyBase);
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

    if (!reply->isError()) {
        d << " success data=" << reply->data();
    } else {
        d << " errorCode=" << reply->networkError() << " ";
        d << " errorString=" << reply->errorString() << " ";
        d << " errorData=" << reply->data() << " ";
    }
    d << "backendStatus=" << reply->backendStatus();
    d << ")";
    return d.space();
}

EnginioReplyBase::EnginioReplyBase(EnginioClientConnectionPrivate *parent, QNetworkReply *reply, EnginioReplyBasePrivate *priv)
    : QObject(*priv, parent->q_ptr)
{
    parent->registerReply(reply, this);
}

EnginioReplyBase::~EnginioReplyBase()
{
    Q_D(EnginioReplyBase);
    Q_ASSERT(d->_nreply->parent() == this);
    if (Q_UNLIKELY(!d->isFinished())) {
        QObject::connect(d->_nreply, &QNetworkReply::finished, d->_nreply, &QNetworkReply::deleteLater);
        d->_client->unregisterReply(d->_nreply);
        d->_nreply->setParent(d->_nreply->manager());
        d->_nreply->abort();
    }
}

/*!
  \internal
*/
QJsonObject EnginioReplyBase::data() const
{
    Q_D(const EnginioReplyBase);
    return d->data();
}

QT_END_NAMESPACE

#endif // QT_NO_DEBUG_STREAM
