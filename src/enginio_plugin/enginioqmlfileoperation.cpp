#include "enginioqmlfileoperation.h"
#include "enginioplugin.h"

#include <QQmlEngine>

/*!
 * \qmltype FileOperation
 * \instantiates EnginioQmlFileOperation
 * \inqmlmodule enginio-plugin
 * \brief Operation for uploading files from local file system to Enginio
 *        backend.
 */

/*!
 * \qmlproperty Client FileOperation::client
 * Enginio client object. This property must be set before the operation is
 * executed.
 */

/*!
 * \qmlproperty UploadStatus FileOperation::uploadStatus
 * Status of the upload operation.
 */

/*!
 * \qmlproperty string FileOperation::fileId
 * Uploaded file's ID or empty string if file has not been uploaded.
 */

/*!
 * \qmlproperty object FileOperation::object
 * Enginio object which is linked to uploaded file or undefined object if file
 * has not been uploaded or file was uploaded without reference to Enginio
 * object. Object contains only id and objectType properties.
 */

/*!
 * \qmlsignal FileOperation::finished()
 *
 * Emitted when the operation completes execution.
 */

/*!
 * \qmlsignal FileOperation::error(Error error)
 *
 * Emitted when an error occurs while the operation is being executed. \a error contains
 * the error details.
 */

/*!
 * \qmlsignal FileOperation::uploadStatusChanged()
 *
 * Emitted when uploadStatus property changes.
 */

/*!
 * \qmlmethod void FileOperation::execute()
 * Execute operation asynchronously. When the operation finishes, \c finished signal
 * is emitted. If there's an error, both \c error and \c finished signals are
 * emitted.
 */

/*!
 * \qmlmethod void FileOperation::cancel()
 * Cancel ongoing operation. \c error signal will be emitted with
 * the \c networkError, \c QNetworkReply::OperationCanceledError.
 */

/*!
 * \qmlmethod string FileOperation::requestParam(string name)
 * Get request parameter with \a name. If request parameter with \a name has not
 * been set, returns empty string.
 */

/*!
 * \qmlmethod void FileOperation::setRequestParam(string name, string value)
 * Set request parameter with \a name and \a value to be added to request URL. If
 * request parameter with same \a name has already been set, the old value will
 * be overwritten. Setting parameter with empty \a value will remove already set
 * parameter.
 *
 * Refer to the Enginio REST API documentation for valid parameters and value
 * syntax.
 */

/*!
 * \qmlmethod string FileOperation::upload(string filePath, string contentType,
 * string objectId, string objectType, bool uploadInChunks)
 *
 * Load file from local file system at \a filePath and upload it to backend.
 * Define file type with \a contentType (for example \c "image/png" ).
 * \a objectId and \a objectType define Enginio object which is contains
 * reference to uploaded file.
 */

/*!
 * \qmlproperty enumeration FileOperation::UploadStatus
 * \list
 * \li FileOperation.UploadStatusUnknown - Upload has not started yet.
 * \li FileOperation.UploadStatusEmpty - In chunked upload mode, when new file
 *     reference has been created but no data chunks have been uploaded.
 * \li FileOperation.UploadStatusIncomplete - In chunked upload mode, when some
 *     (but not all) data chunks have been uploaded.
 * \li FileOperation.UploadStatusComplete - File has been uploaded completely.
 * \endlist
 */
EnginioQmlFileOperation::EnginioQmlFileOperation(EnginioQmlClient *client,
                                                 QObject *parent) :
    EnginioFileOperation(client, parent)
{
}

EnginioQmlClient * EnginioQmlFileOperation::getClient() const
{
    return static_cast<EnginioQmlClient*>(client());
}

QJSValue EnginioQmlFileOperation::object() const
{
    QJSValue object = g_qmlEngine->newObject();
    object.setProperty(QStringLiteral("id"), objectId());
    object.setProperty(QStringLiteral("objectType"), objectType());
    return object;
}
