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

#ifndef ENGINIOFILEOPERATION_H
#define ENGINIOFILEOPERATION_H

#include "enginiooperation.h"
#include <QIODevice>

class EnginioFileOperationPrivate;

class ENGINIOCLIENT_EXPORT EnginioFileOperation : public EnginioOperation
{
    Q_OBJECT
    Q_ENUMS(UploadStatus)
    Q_PROPERTY(UploadStatus uploadStatus READ uploadStatus NOTIFY uploadStatusChanged)
    Q_PROPERTY(double uploadProgress READ uploadProgress NOTIFY uploadProgressChanged)

public:
    enum UploadStatus {
        UploadStatusUnknown = 0,
        UploadStatusEmpty,
        UploadStatusIncomplete,
        UploadStatusComplete
    };

    explicit EnginioFileOperation(EnginioClient *client,
                                  QObject *parent = 0);
    virtual ~EnginioFileOperation();

    QString fileId() const;
    UploadStatus uploadStatus() const;
    void upload(QIODevice *data,
                const QString &fileName,
                const QString &contentType,
                const QString &objectType,
                const QString &objectId = QString(),
                qint64 chunkSize = Q_INT64_C(1024*1024));

    Q_INVOKABLE void upload(const QString &filePath,
                            const QString &contentType,
                            const QString &objectType,
                            const QString &objectId = QString(),
                            qint64 chunkSize = Q_INT64_C(1024*1024));

    QString objectId() const;
    QString objectType() const;
    double uploadProgress() const;

signals:
    void uploadStatusChanged() const;
    void uploadProgressChanged() const;

protected:
    EnginioFileOperation(EnginioClient *client,
                         EnginioFileOperationPrivate &dd,
                         QObject *parent = 0);

private:
    Q_DECLARE_PRIVATE(EnginioFileOperation)
    Q_DISABLE_COPY(EnginioFileOperation)
};

#endif // ENGINIOFILEOPERATION_H
