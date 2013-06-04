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

#include "enginioerror_p.h"
#include <QDebug>

/*!
 * \class EnginioError
 * \inmodule enginio-client
 * \brief Used to describe errors that occur during Enginio operations
 */

/*!
 * \enum EnginioError::ErrorType
 * \value NoError
 *        No errors.
 * \value UnknownError
 *        Something went wrong but we don't know what.
 * \value NetworkError
 *        Enginio service is unavailable or can't handle request.
 * \value RequestError
 *        Request or reply is invalid.
 * \value InternalError
 *        Enginio service is malfunctioning.
 */

EnginioErrorPrivate::EnginioErrorPrivate(EnginioError *error) :
    q_ptr(error),
    m_error(EnginioError::NoError),
    m_networkError(QNetworkReply::NoError),
    m_httpCode(0)
{
}

EnginioErrorPrivate::~EnginioErrorPrivate()
{
}


/*!
 * Create a new error object with the \a error type, \a errorString,
 * optional \a networkError code, return \a httpCode code, and
 * a \a parent object.
 */
EnginioError::EnginioError(ErrorType error,
                           QString errorString,
                           QNetworkReply::NetworkError networkError,
                           int httpCode,
                           QObject *parent) :
    QObject(parent),
    d_ptr(new EnginioErrorPrivate(this))
{
    Q_D(EnginioError);
    d->m_error = error;
    d->m_errorString = errorString;
    d->m_networkError = networkError;
    d->m_httpCode = httpCode;
}

/*!
 * Constructor used in inheriting classes.
 * \internal
 */
EnginioError::EnginioError(ErrorType error,
                           QString errorString,
                           EnginioErrorPrivate &dd,
                           QObject *parent) :
    QObject(parent),
    d_ptr(&dd)
{
    Q_D(EnginioError);
    d->m_error = error;
    d->m_errorString = errorString;
}

/*!
 * Destructor.
 */
EnginioError::~EnginioError()
{
    delete d_ptr;
}

/*!
 * Error type.
 */
EnginioError::ErrorType EnginioError::error() const
{
    Q_D(const EnginioError);
    return d->m_error;
}

/*!
 * Description of the error.
 */
QString EnginioError::errorString() const
{
    Q_D(const EnginioError);
    return d->m_errorString;
}

/*!
 * If error is \c NetworkError this returns the type of network error.
 */
QNetworkReply::NetworkError EnginioError::networkError() const
{
    Q_D(const EnginioError);
    return d->m_networkError;
}

/*!
 * HTTP reply code. If there's no error, this will return 0.
 */
int EnginioError::httpCode() const
{
    Q_D(const EnginioError);
    return d->m_httpCode;
}

void EnginioError::setError(ErrorType type)
{
    Q_D(EnginioError);
    d->m_error = type;
}

void EnginioError::setErrorString(const QString &errorString)
{
    Q_D(EnginioError);
    d->m_errorString = errorString;
}

void EnginioError::setNetworkError(QNetworkReply::NetworkError networkError)
{
    Q_D(EnginioError);
    d->m_networkError = networkError;
}

void EnginioError::setHttpCode(int httpCode)
{
    Q_D(EnginioError);
    d->m_httpCode = httpCode;
}
