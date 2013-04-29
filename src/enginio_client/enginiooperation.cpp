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

#include "enginiooperation_p.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

/*!
 * \class EnginioOperation
 * \inmodule enginio-client
 * \brief Base class for the Enginio operations
 * \sa EnginioObjectOperation, EnginioQueryOperation
 */

/*!
 * \enum EnginioOperation::State
 * \value StateNotStarted
 *        Operation has not been executed yet.
 * \value StateExecuting
 *        Operation has been executed but execution has not finished yet.
 * \value StateFinished
 *        Operation has been executed and it has finished (but not canceled).
 * \value StateCanceled
 *        Operation has been executed and it has been canceled by user. Please
 *        note that when \c cancel() is called \c finished signal will be
 *        emitted but operation state will be \c StateCanceled.
 */

/*!
 * \fn void EnginioOperation::finished() const
 *
 * Emitted operation execution finishes.
 */

/*!
 * \fn void EnginioOperation::error(EnginioError *error) const
 *
 * Emitted when error happens in operation execution. \a error contains error
 * details.
 */

/*!
 * \fn void EnginioOperation::stateChanged(State newState) const
 *
 * Emitted when operation state changes. New state is in \a newState.
 */

EnginioOperationPrivate::EnginioOperationPrivate(EnginioOperation *op) :
    q_ptr(op),
    m_client(0),
    m_error(new EnginioError()),
    m_reply(0),
    m_state(EnginioOperation::StateNotStarted)
{
}

EnginioOperationPrivate::~EnginioOperationPrivate()
{
}

QUrlQuery EnginioOperationPrivate::urlQuery() const
{
    QUrlQuery query;

    QMapIterator<QString, QString> i(m_requestParams);
    while (i.hasNext()) {
        i.next();
        query.addQueryItem(i.key(), i.value());
    }

    return query;
}

QNetworkRequest EnginioOperationPrivate::enginioRequest(const QUrl &url)
{
    QNetworkRequest req(url);
    req.setRawHeader("Enginio-Backend-Id",
                     m_client->backendId().toLatin1());
    req.setRawHeader("Enginio-Backend-Secret",
                     m_client->backendSecret().toLatin1());
    QString sessionToken = m_client->sessionToken();
    if (!sessionToken.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Using session token" << sessionToken;
        req.setRawHeader("Enginio-Backend-Session", sessionToken.toLatin1());
    }
    return req;
}

/*!
 * \brief Set error and emit \c error signal
 */
void EnginioOperationPrivate::setError(EnginioError::ErrorType type,
                                       const QString &description,
                                       int httpCode,
                                       QNetworkReply::NetworkError networkError)
{
    Q_Q(EnginioOperation);

    qDebug() << "setError:" << description << "; http:"<< httpCode <<
                "; network:" << networkError;

    m_error->setError(type);
    m_error->setErrorString(description);
    m_error->setNetworkError(networkError);
    m_error->setHttpCode(httpCode);

    if (type != EnginioError::NoError)
        emit q->error(m_error);
}

void EnginioOperationPrivate::onRequestFinished()
{
    Q_Q(EnginioOperation);

    if (!m_reply.isNull() && m_reply->error() == QNetworkReply::NoError)
        handleResults();

    if (!m_reply.isNull())
        m_reply->deleteLater();

    if (isFinished() || m_state == EnginioOperation::StateCanceled) {
        if (m_state != EnginioOperation::StateCanceled) {
            m_state = EnginioOperation::StateFinished;
            emit q->stateChanged(m_state);
        }
        emit q->finished();

    } else {
        m_reply = doRequest(m_client->apiUrl());

        if (!m_reply.isNull()) {
            qDebug() << "=== Request" << this << m_reply->operation() << m_reply->url();
            connect(m_reply, SIGNAL(finished()),
                    this, SLOT(onRequestFinished()),
                    Qt::QueuedConnection);
            connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
                    this, SLOT(onRequestError(QNetworkReply::NetworkError)),
                    Qt::QueuedConnection);
        }
    }
}

void EnginioOperationPrivate::onRequestError(QNetworkReply::NetworkError error)
{
    QString errorString;

    QByteArray data = m_reply->readAll();
    qDebug() << "=== Error reply" << q_ptr << m_reply->operation() << m_reply->url();
    qDebug() << data;
    qDebug() << "=== Error reply end ===";

    if (!data.isEmpty()) {
        QJsonDocument replyDoc = QJsonDocument::fromJson(data);
        if (replyDoc.isObject()) {
            QJsonObject replyObject = replyDoc.object();
            errorString = replyObject.value(QStringLiteral("errors")).toArray().first()
                    .toObject().value(QStringLiteral("message")).toString();
        }
    }
    if (errorString.isEmpty())
        errorString = QStringLiteral("Network error");

    setError(EnginioError::NetworkError,
             errorString,
             m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
             error);
}


/*!
 * Constructor used in inheriting classes.
 */
EnginioOperation::EnginioOperation(EnginioClient *client,
                                   EnginioOperationPrivate &dd,
                                   QObject *parent) :
    QObject(parent),
    d_ptr(&dd)
{
    d_ptr->m_client = client;
}

/*!
 * Destructor.
 */
EnginioOperation::~EnginioOperation()
{
    delete d_ptr;
}

/*!
 * Return the EnginioClient pointer set for operation.
 */
EnginioClient * EnginioOperation::client() const
{
    Q_D(const EnginioOperation);
    return d->m_client;
}

/*!
 * Change operation's EnginioClient pointer to \a client. Don't call this method
 * on active operation.
 */
void EnginioOperation::setClient(EnginioClient *client)
{
    Q_D(EnginioOperation);
    d->m_client = client;
}

/*!
 * Returns the error that was emitted with last \c error signal. If no error
 * has been detected, returns error with type \c EnginioError::NoError.
 */
EnginioError *EnginioOperation::error() const
{
    Q_D(const EnginioOperation);
    return d->m_error;
}

/*!
 * Operation state.
 */
EnginioOperation::State EnginioOperation::state() const
{
    Q_D(const EnginioOperation);
    return d->m_state;
}

/*!
 * Get request parameter with \a name. If request parameter with \a name has not
 * been set, returns empty string.
 */
QString EnginioOperation::requestParam(const QString &name) const
{
    Q_D(const EnginioOperation);
    return d->m_requestParams.value(name);
}

/*!
 * Set request parameter with \a name and \a value to be added to request URL. If
 * request parameter with same \a name has already been set, the old value will
 * be overwritten. Setting parameter with empty \a value will remove already set
 * parameter.
 *
 * Refer to the Enginio REST API documentation for valid parameters and value
 * syntax.
 */
void EnginioOperation::setRequestParam(const QString &name,
                                       const QString &value)
{
    Q_D(EnginioOperation);
    if (value.isEmpty())
        d->m_requestParams.remove(name);
    else
        d->m_requestParams.insert(name, value);
}

/*!
 * Execute operation asynchronously. When operation finishes \c finished signal
 * is emitted. If there's an error, both \c error and \c finished signals are
 * emitted.
 */
void EnginioOperation::execute()
{
    Q_D(EnginioOperation);

    if (Q_UNLIKELY(d->m_state == StateExecuting)) {
        qWarning() << Q_FUNC_INFO << "Already executing";
        return;
    }

    if (Q_UNLIKELY(!d->m_client)) {
        qWarning() << Q_FUNC_INFO << "Unknown client";
        d->setError(EnginioError::RequestError, QStringLiteral("Unknown client"));
        emit finished();
        return;
    }

    if (!d->m_reply.isNull()) {
        d->m_reply->deleteLater();
    }

    d->reset();

    d->m_reply = d->doRequest(d->m_client->apiUrl());

    if (!d->m_reply.isNull()) {
        qDebug() << "=== Request" << this << d->m_reply->operation() << d->m_reply->url();
        connect(d->m_reply, SIGNAL(finished()),
                d, SLOT(onRequestFinished()),
                Qt::QueuedConnection);
        connect(d->m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
                d, SLOT(onRequestError(QNetworkReply::NetworkError)),
                Qt::QueuedConnection);
        d->m_state = StateExecuting;
        emit stateChanged(d->m_state);
    }
}

/*!
 * Cancel ongoing operation. \c error signal will be emitted with
 * \c networkError \c QNetworkReply::OperationCanceledError.
 */
void EnginioOperation::cancel()
{
    Q_D(EnginioOperation);
    if (!d->m_reply.isNull()) {
        qDebug() << "Cancelling request" << this << d->m_reply->operation() << d->m_reply->url();
        d->m_reply->abort();
        d->m_state = StateCanceled;
        emit stateChanged(d->m_state);
    }
}
