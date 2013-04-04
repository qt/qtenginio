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

#include "enginiofileoperation_p.h"
#include <QDebug>
#include <QHttpPart>
#include <QJsonDocument>
#include <QJsonObject>

/*!
 * \class EnginioFileOperation
 * \inmodule enginio-client
 * \brief Operation for uploading and downloading binary files to/from Enginio
 *        backend
 */

EnginioFileOperationPrivate::EnginioFileOperationPrivate(EnginioFileOperation *op) :
    EnginioOperationPrivate(op),
    m_type(NullFileOperation),
    m_fileDevice(0)
{
}

EnginioFileOperationPrivate::~EnginioFileOperationPrivate()
{
    if (m_multiPart)
        delete m_multiPart;
}

/*!
 * URL path for operation
 */
QString EnginioFileOperationPrivate::requestPath() const
{
    if (m_type == UploadMultipartOperation)
        return "/v1/files";

    if (m_type == UploadChunkedOperation)
        qWarning() << "UploadChunkedOperation not implemented";

    return QString();
}

/*!
 * Generate and execute backend request for this operation. \a backendUrl is the
 * base URL (protocol://host:port) for backend. Returns a new QNetworkReply
 * object.
 */
QNetworkReply * EnginioFileOperationPrivate::doRequest(const QUrl &backendUrl)
{
    Q_Q(EnginioFileOperation);

    QString path = requestPath();
    QString error;

    if (m_type == NullFileOperation)
        error = "Unknown operation type";
    else if (path.isEmpty())
        error = "Request URL creation failed";

    if (!error.isEmpty()) {
        setError(EnginioError::RequestError, error);
        emit q->finished();
        return 0;
    }

    QUrl url(backendUrl);
    url.setPath(path);
    url.setQuery(urlQuery());

    QNetworkRequest req = enginioRequest(url);
    QNetworkAccessManager *netManager = m_client->networkManager();

    switch (m_type) {
    case UploadMultipartOperation:
        if (!m_fileDevice->isReadable()) {
            setError(EnginioError::RequestError,
                     QStringLiteral("File is not open for reading"));
            emit q->finished();
            return 0;
        }
        return netManager->post(req, m_multiPart);
    case UploadChunkedOperation:
        qWarning() << "UploadChunkedOperation not implemented";
        return 0;
    default:
        return 0;
    }
}

void EnginioFileOperationPrivate::handleResults()
{
    QByteArray data = m_reply->readAll();

    qDebug() << "=== Reply" << q_ptr << m_reply->operation() << m_reply->url();
    qDebug() << data;
    qDebug() << "=== Reply end ===";

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        setError(EnginioError::RequestError, QStringLiteral("Invalid reply data"));
        return;
    }

    QJsonObject json = doc.object();
    QString id = json.value(QStringLiteral("id")).toString();

    if (id.isEmpty()) {
        setError(EnginioError::InternalError,
                 QStringLiteral("Unknown ID on returned object"));
        return;
    }

    m_fileId = id;
    m_uploadStatus = json.value(QStringLiteral("status")).toString();
}

QByteArray EnginioFileOperationPrivate::requestMetadata(bool includeFileName) const
{
    qDebug() << Q_FUNC_INFO;

    QByteArray json;
    bool objectWritten = false;

    json += '{';

    if (!m_objectId.isEmpty() && !m_objectType.isEmpty()) {
        json += "\"object\":{\"id\":\"";
        json += m_objectId;
        json += "\",\"objectType\":\"";
        json += m_objectType;
        json += "\"}";
        objectWritten = true;
    }

    if (includeFileName && !m_fileName.isEmpty()) {
        if (objectWritten)
            json += ',';
        json += "\"fileName\":\"";
        json += m_fileName;
        json += '"';
    }

    json += '}';
    qDebug() << json;

    return json;
}

/*!
 * Create new file operation. \a client must be a valid pointer to the EnginioClient
 * object. \a parent is optional.
 */
EnginioFileOperation::EnginioFileOperation(EnginioClient *client,
                                           QObject *parent) :
    EnginioOperation(client, *new EnginioFileOperationPrivate(this), parent)
{
    qDebug() << this << "created";
}

/*!
 * Constructor used in inheriting classes.
 */
EnginioFileOperation::EnginioFileOperation(EnginioClient *client,
                                           EnginioFileOperationPrivate &dd,
                                           QObject *parent) :
    EnginioOperation(client, dd, parent)
{
    qDebug() << this << "created";
}

/*!
 * Destructor.
 */
EnginioFileOperation::~EnginioFileOperation()
{
    qDebug() << this << "deleted";
}

QString EnginioFileOperation::fileId() const
{
    Q_D(const EnginioFileOperation);
    return d->m_fileId;
}

QString EnginioFileOperation::uploadStatus() const
{
    Q_D(const EnginioFileOperation);
    return d->m_uploadStatus;
}

/*!
 * Uploads file to Enginio backend. File is read from QIODevice \a data which
 * must be open for reading when operation is executed and must remain valid
 * until the finished() signal is emitted.
 */
void EnginioFileOperation::upload(const QString &fileName,
                                  const QString &contentType,
                                  const QString &objectId,
                                  const QString &objectType,
                                  QIODevice *data,
                                  bool uploadInChunks)
{
    if (uploadInChunks) {
        qWarning() << "uploadInChunks not implemented";
        return;
    }

    Q_D(EnginioFileOperation);
    d->m_fileName = fileName;
    d->m_contentType = contentType;
    d->m_objectId = objectId;
    d->m_objectType = objectType;
    d->m_fileDevice = data;

    d->m_type = EnginioFileOperationPrivate::UploadMultipartOperation;

    if (d->m_multiPart)
        delete d->m_multiPart;
    d->m_multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart objectPart;
    objectPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                         QVariant("form-data; name=\"object\""));
    objectPart.setBody(d->requestMetadata(false));

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, d->m_contentType);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileName));
    filePart.setBodyDevice(data);

    d->m_multiPart->append(objectPart);
    d->m_multiPart->append(filePart);
}
