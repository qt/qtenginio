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

#ifndef ENGINIOOPERATION_H
#define ENGINIOOPERATION_H

#include "enginioclient_global.h"
#include <QObject>

class EnginioClient;
class EnginioError;
class EnginioOperationPrivate;

class ENGINIOCLIENT_EXPORT EnginioOperation : public QObject
{
    Q_OBJECT
    Q_ENUMS(State)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)

public:
    enum State {
        StateNotStarted = 0,
        StateExecuting,
        StateFinished,
        StateCanceled
    };

    virtual ~EnginioOperation();

    EnginioClient * client() const;
    EnginioError * error() const;
    State state() const;
    Q_INVOKABLE QString requestParam(const QString &name) const;
    Q_INVOKABLE void setRequestParam(const QString &name, const QString &value);

public slots:
    void execute();
    void cancel();

signals:
    void finished() const;
    void error(EnginioError *error) const;
    void stateChanged(State newState) const;

protected:
    EnginioOperationPrivate *d_ptr;
    EnginioOperation(EnginioClient *client,
                     EnginioOperationPrivate &dd,
                     QObject *parent = 0);

    void setClient(EnginioClient *client);

private:
    Q_DECLARE_PRIVATE(EnginioOperation)
};

#endif // ENGINIOOPERATION_H
