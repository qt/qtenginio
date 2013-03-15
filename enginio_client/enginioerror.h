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

#ifndef ENGINIOERROR_H
#define ENGINIOERROR_H

#include "enginioclient_global.h"
#include <QObject>
#include <QNetworkReply>

class EnginioErrorPrivate;

class ENGINIOCLIENT_EXPORT EnginioError : public QObject
{
    Q_OBJECT
    Q_ENUMS(ErrorType QNetworkReply::NetworkError)
    Q_PROPERTY(ErrorType error READ error)
    Q_PROPERTY(QString errorString READ errorString)
    Q_PROPERTY(QNetworkReply::NetworkError networkError READ networkError)
    Q_PROPERTY(int httpCode READ httpCode)

public:
    enum ErrorType {
        NoError = 0,
        UnknownError,
        NetworkError,
        RequestError,
        InternalError
    };

    explicit EnginioError(
            ErrorType error = NoError,
            QString errorString = QString(),
            QNetworkReply::NetworkError networkError = QNetworkReply::NoError,
            int httpCode = 0,
            QObject *parent = 0);
    virtual ~EnginioError();

    ErrorType error() const;
    QString errorString() const;
    QNetworkReply::NetworkError networkError() const;
    int httpCode() const;

protected:
    EnginioErrorPrivate *d_ptr;
    EnginioError(ErrorType error,
                 QString errorString,
                 EnginioErrorPrivate &dd,
                 QObject *parent = 0);

private:
    Q_DECLARE_PRIVATE(EnginioError)
};

Q_DECLARE_METATYPE(EnginioError*)

#endif // ENGINIOERROR_H
