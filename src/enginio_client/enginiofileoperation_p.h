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

#ifndef ENGINIOFILEOPERATION_P_H
#define ENGINIOFILEOPERATION_P_H

#include "enginiofileoperation.h"
#include "enginiooperation_p.h"

class EnginioFileOperationPrivate : public EnginioOperationPrivate
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(EnginioFileOperation)

public:
    enum FileOperationType {
        NullFileOperation = 0,
        UploadMultipartOperation,
        UploadChunkedOperation
    };

    EnginioFileOperationPrivate(EnginioFileOperation *op = 0);
    ~EnginioFileOperationPrivate();

    // From EnginioOperationPrivate
    virtual QString requestPath() const;
    virtual QNetworkReply *doRequest(const QUrl &backendUrl);
    virtual void handleResults();
    virtual bool isFinished();
    virtual void reset();

    QByteArray requestMetadata(bool includeFileName) const;
    EnginioFileOperation::UploadStatus uploadStatusFromString(
            const QString &statusString) const;
    QByteArray nextChunk(QString *rangeString = 0);
    bool initializeOperation();

    FileOperationType m_type;
    QString m_fileId;
    QString m_fileName;
    QString m_contentType;
    EnginioFileOperation::UploadStatus m_uploadStatus;
    QString m_objectId;
    QString m_objectType;
    QPointer<QHttpMultiPart> m_multiPart;
    QIODevice *m_fileDevice;
    bool m_fromFile;
    qint64 m_chunkSize;
    qint64 m_bytesUploaded;
    int m_lastChunkSize;
};

#endif // ENGINIOFILEOPERATION_P_H
