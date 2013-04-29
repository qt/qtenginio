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
#include <QFile>
#include <QHttpPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

/*!
 * \class EnginioFileOperation
 * \inmodule enginio-client
 * \brief Operation for uploading binary files to Enginio backend.
 */

/*!
 * \enum EnginioFileOperation::UploadStatus
 * \value UploadStatusUnknown
 *        Upload has not started yet.
 * \value UploadStatusEmpty
 *        In chunked upload mode, when new file reference has been created but
 *        no data chunks have been uploaded.
 * \value UploadStatusIncomplete
 *        In chunked upload mode, when some (but not all) data chunks have been
 *        uploaded.
 * \value UploadStatusComplete
 *        File has been uploaded completely.
 */

/*!
 * \fn void EnginioFileOperation::uploadStatusChanged() const
 *
 * Emitted when file upload status changes.
 */

/*!
 * \fn void EnginioFileOperation::uploadProgressChanged() const
 *
 * Emitted when file upload progress changes.
 */

EnginioFileOperationPrivate::EnginioFileOperationPrivate(EnginioFileOperation *op) :
    EnginioOperationPrivate(op),
    m_type(NullFileOperation),
    m_uploadStatus(EnginioFileOperation::UploadStatusUnknown),
    m_fileDevice(0),
    m_fromFile(false)
{
    m_chunkSize =  Q_INT64_C(1024 * 1024);
    m_bytesUploaded =  Q_INT64_C(0);
    m_lastChunkSize = 0;
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
        return QStringLiteral("/v1/files");

    if (m_type == UploadChunkedOperation) {
        if (m_fileId.isEmpty())
            return "/v1/files";
        return QString("/v1/files/%1/chunk").arg(m_fileId);
    }

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

    if (m_type == NullFileOperation && !initializeOperation()) {
        emit q->finished();
        return 0;
    }

    QString path = requestPath();
    QString error;

    if (m_type == NullFileOperation)
        error = QStringLiteral("Unknown operation type");
    else if (path.isEmpty())
        error = QStringLiteral("Request URL creation failed");
    else if (m_fromFile && m_fileDevice && !m_fileDevice->isOpen()) {
        if (!m_fileDevice->open(QIODevice::ReadOnly))
            error = QStringLiteral("Can't open file for reading");
    }

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

    if (!m_fileDevice->isReadable()) {
        setError(EnginioError::RequestError,
                 QStringLiteral("File is not open for reading"));
        emit q->finished();
        return 0;
    }

    QByteArray data;
    QString range;

    switch (m_type) {
    case UploadMultipartOperation:
        return netManager->post(req, m_multiPart);
    case UploadChunkedOperation:
        if (m_fileId.isEmpty()) {
            req.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/json"));
            return netManager->post(req, requestMetadata(true));
        }
        data = nextChunk(&range);
        if (data.isEmpty())
            return 0;

        req.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/octet-stream"));
        req.setRawHeader("Content-Range", range.toLatin1());
        m_lastChunkSize = data.size();
        return netManager->put(req, data);
    default:
        break;
    }

    return 0;
}

void EnginioFileOperationPrivate::handleResults()
{
    Q_Q(EnginioFileOperation);

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

    EnginioFileOperation::UploadStatus oldStatus = m_uploadStatus;
    m_uploadStatus = uploadStatusFromString(
                json.value(QStringLiteral("status")).toString());
    if (oldStatus != m_uploadStatus) {
        emit q->uploadStatusChanged();
    }

    m_bytesUploaded += m_lastChunkSize;
    if (m_lastChunkSize > 0 || m_type == UploadMultipartOperation)
        emit q->uploadProgressChanged();

    if (m_fromFile && m_fileDevice && isFinished())
        m_fileDevice->close();
}

bool EnginioFileOperationPrivate::isFinished()
{
    return m_uploadStatus == EnginioFileOperation::UploadStatusComplete;
}

void EnginioFileOperationPrivate::reset()
{
    Q_Q(EnginioFileOperation);

    m_type = NullFileOperation;
    m_bytesUploaded = Q_INT64_C(0);
    m_lastChunkSize = 0;

    EnginioFileOperation::UploadStatus oldStatus = m_uploadStatus;
    m_uploadStatus = EnginioFileOperation::UploadStatusUnknown;
    if (oldStatus != m_uploadStatus) {
        emit q->uploadStatusChanged();
    }

    if (m_fileDevice) {
        if (m_fromFile) {
            if (m_fileDevice->isOpen())
                m_fileDevice->close();
        } else {
            if (m_fileDevice->isOpen())
                m_fileDevice->seek(Q_INT64_C(0));
        }
    }
}

QByteArray EnginioFileOperationPrivate::requestMetadata(bool includeFileName) const
{
    QByteArray json;
    bool objectWritten = false;

    json += '{';

    if (!m_objectType.isEmpty()) {
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
    return json;
}

EnginioFileOperation::UploadStatus EnginioFileOperationPrivate::uploadStatusFromString(
        const QString &statusString) const
{
    if (statusString == QStringLiteral("empty"))
        return EnginioFileOperation::UploadStatusEmpty;
    if (statusString == QStringLiteral("incomplete"))
        return EnginioFileOperation::UploadStatusIncomplete;
    if (statusString == QStringLiteral("complete"))
        return EnginioFileOperation::UploadStatusComplete;
    return EnginioFileOperation::UploadStatusUnknown;
}

QByteArray EnginioFileOperationPrivate::nextChunk(QString *rangeString)
{
    QByteArray data;

    if (m_fileDevice && m_fileDevice->isReadable())
        data = m_fileDevice->read(m_chunkSize);

    if (rangeString) {
        rangeString->clear();
        rangeString->append(QString("%1-%2/%3").arg(m_bytesUploaded)
                            .arg(m_bytesUploaded + data.size())
                            .arg(m_fileDevice->size()));
    }

    return data;
}

bool EnginioFileOperationPrivate::initializeOperation()
{
    if (!m_fileDevice) {
        setError(EnginioError::RequestError, "No data to upload");
        return false;
    }

    if (m_fromFile) {
        if (!m_fileDevice->isOpen()) {
            if (!m_fileDevice->open(QIODevice::ReadOnly)) {
                setError(EnginioError::RequestError, "Can't open file for reading");
                return false;
            }
        }
    } else if (!m_fileDevice->isOpen()){
        setError(EnginioError::RequestError, "File is not open for reading");
        return false;
    }

    qint64 fileSize = m_fileDevice->size();
    if (fileSize == Q_INT64_C(0) || fileSize < m_chunkSize) {
        // If file size is unknown or smaller than one chunk, we upload whole
        // file in one request.
        m_type = EnginioFileOperationPrivate::UploadMultipartOperation;

        if (m_multiPart)
            delete m_multiPart;
        m_multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        if (!m_objectType.isEmpty()) {
            QHttpPart objectPart;
            objectPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                 QVariant("form-data; name=\"object\""));
            objectPart.setBody(requestMetadata(false));
            m_multiPart->append(objectPart);
        }

        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, m_contentType);
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QString("form-data; name=\"file\"; filename=\"%1\"").arg(m_fileName));
        filePart.setBodyDevice(m_fileDevice);
        m_multiPart->append(filePart);

    } else {
        m_type = EnginioFileOperationPrivate::UploadChunkedOperation;
    }

    return true;
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

/*!
 * Returns the ID of the file object.
 */
QString EnginioFileOperation::fileId() const
{
    Q_D(const EnginioFileOperation);
    return d->m_fileId;
}

/*!
 * Returns the status of file being uploaded.
 */
EnginioFileOperation::UploadStatus EnginioFileOperation::uploadStatus() const
{
    Q_D(const EnginioFileOperation);
    return d->m_uploadStatus;
}

/*!
 * Uploads file to Enginio backend.
 *
 * \list
 * \li \a data is the QIODevice which is be used to read file data. This device
 *     must be open for reading when operation is executed and must remain valid
 *     until the finished() signal is emitted.
 * \li \a fileName is the name of the file including possible extension.
 * \li \a contentType describes the type of the file (for example "image/jpeg").
 * \li \a objectType and \a objectId describe existing Enginio object which
 *     refernces uploaded file. Each uploaded file must be referenced by an
 *     object but you can give empty \a objectId if referencing object is not
 *     yet created.
 * \li If \a chunkSize is smaller than size of the file, file is uploaded in two
 *     or more parts.
 * \endlist
 */

void EnginioFileOperation::upload(QIODevice *data,
                                  const QString &fileName,
                                  const QString &contentType,
                                  const QString &objectType,
                                  const QString &objectId,
                                  qint64 chunkSize)
{
    qDebug() << Q_FUNC_INFO << fileName << contentType << objectType << objectId << chunkSize;

    Q_D(EnginioFileOperation);
    d->m_fileDevice = data;
    d->m_fileName = fileName;
    d->m_contentType = contentType;
    d->m_objectType = objectType;
    d->m_objectId = objectId;
    d->m_chunkSize = chunkSize;
}

/*!
 * Uploads file to Enginio backend.
 *
 * \list
 * \li \a filePath is path pointing to file in local file system. Last part of
 *     the path (everything after last '/') will be used as file name.
 * \li \a contentType describes the type of the file (for example "image/jpeg").
 * \li \a objectType and \a objectId describe existing Enginio object which
 *     refernces uploaded file. Each uploaded file must be referenced by an
 *     object but you can give empty \a objectId if referencing object is not
 *     yet created.
 * \li If \a chunkSize is smaller than size of the file, file is uploaded in two
 *     or more parts.
 * \endlist
 */

void EnginioFileOperation::upload(const QString &filePath,
                                  const QString &contentType,
                                  const QString &objectType,
                                  const QString &objectId,
                                  qint64 chunkSize)
{
    qDebug() << Q_FUNC_INFO << filePath << contentType << objectType << objectId << chunkSize;

    Q_D(EnginioFileOperation);
    d->m_fromFile = true;

    upload(new QFile(filePath),
           filePath.split('/').last(),
           contentType,
           objectType,
           objectId,
           chunkSize);
}

/*!
 * Returns the ID of the referenced Enginio object.
 */
QString EnginioFileOperation::objectId() const
{
    Q_D(const EnginioFileOperation);
    return d->m_objectId;
}

/*!
 * Returns the type of the referenced Enginio object.
 */
QString EnginioFileOperation::objectType() const
{
    Q_D(const EnginioFileOperation);
    return d->m_objectType;
}

/*!
 * Returns the upload progress as percentage.
 */
double EnginioFileOperation::uploadProgress() const
{
    Q_D(const EnginioFileOperation);

    switch (d->m_uploadStatus) {
    case UploadStatusComplete:
        return 100.0;
    case UploadStatusIncomplete:
        if (d->m_fileDevice)
            return (double)d->m_bytesUploaded / d->m_fileDevice->size();
    default:
        break;
    }
    return 0.0;
}
