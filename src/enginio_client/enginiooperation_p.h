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

#ifndef ENGINIOOPERATION_P_H
#define ENGINIOOPERATION_P_H

#include "enginiooperation.h"
#include "enginioerror.h"
#include "enginioclient.h"
#include <QNetworkReply>
#include <QPointer>
#include <QUrlQuery>

class EnginioOperationPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(EnginioOperation)

public:
    EnginioOperationPrivate(EnginioOperation *op = 0);
    virtual ~EnginioOperationPrivate();
    EnginioOperation *q_ptr;

    virtual QString requestPath() const = 0;
    virtual QUrlQuery urlQuery() const;
    virtual QNetworkReply * doRequest(const QUrl &backendUrl) = 0;
    virtual void handleResults() = 0;
    virtual QNetworkRequest enginioRequest(const QUrl &url);

    void setError(EnginioError::ErrorType type,
                  const QString &description,
                  int httpCode = 0,
                  QNetworkReply::NetworkError networkError = QNetworkReply::NoError);

    QPointer<EnginioClient> m_client;
    QPointer<EnginioError> m_error;
    QPointer<QNetworkReply> m_reply;
    EnginioOperation::State m_state;
    QMap<QString, QString> m_requestParams;

public slots:
    void onRequestFinished();
    void onRequestError(QNetworkReply::NetworkError error);
};

#endif // ENGINIOOPERATION_P_H
