#include "enginioqmlfileoperation.h"
#include "enginioplugin.h"

#include <QQmlEngine>

/*!
 * \qmltype FileOperation
 * \instantiates EnginioQmlFileOperation
 * \inqmlmodule enginio-plugin
 * \brief Operation for uploading files from local file system to Enginio
 *        backend.
 *
 * Files in Enginio are handled as object references in regular objects. For
 * example you might have object for product:
 *
 * \code
 * {
 *     "id": "517a244a989e97145b013288",
 *     "objectType": "objects.product",
 *     "name": "Pants"
 * }
 * \endcode
 *
 * In order to add image for this product you need to upload the image file with
 * FileOperation and then add reference to file object in product object. For
 * example:
 *
 * \code
 * Component.onCompleted: {
 *     fileUploader.upload("file://pants.png", "image/png",
 *                         "objects.product", "517a244a989e97145b013288");
 *     fileUploader.execute();
 * }
 *
 * Enginio.FileOperation {
 *     id: fileUploader
 *     client: client
 *     onFinished: {
 *         // Add image reference to product object
 *         var object = fileUploader.object;
 *         object.image = {
 *             "objectType": "files",
 *             "id": fileUploader.fileId
 *         }
 *         productUpdater.update(object);
 *         productUpdater.execute();
 *     }
 * }
 *
 * Enginio.ObjectOperation {
 *     id: productUpdater
 *     client: client
 * }
 * \endcode
 *
 * After this you might query product objects like this:
 *
 * \code
 * Enginio.QueryOperation {
 *     id: queryOperation
 *     client: client
 *     model: productModel
 *     objectTypes: ["objects.product"]
 *     // Get full image object, not just id and objectType
 *     include: {"image": {}}
 * }
 * \endcode
 *
 * Query result objects look something like this:
 *
 * \code
 * {
 *     "id": "517a244a989e97145b013288",
 *     "objectType": "objects.product",
 *     "name": "Pants",
 *     "image": {
 *         "id": "517a244a989e97145b01328d",
 *         "objectType": "files".
 *         "fileName": "pants.png",
 *         "fileSize": 7679,
 *         "contentType": "image/png",
 *         "url":"/v1/download/515d6db45a3d8b1312018fd8/original/pants.png",
 *         ...
 *     },
 *     ...
 * }
 * \endcode
 *
 * Image can be downloaded using \c url property of the file object. It should
 * be prefixed with \c apiUrl property of Enginio Client.
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
 * \qmlproperty number FileOperation::uploadProgress
 * Upload progress as percentage.
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
 * \qmlsignal FileOperation::uploadProgressChanged()
 *
 * Emitted when upload progress changes.
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
 * string objectType, string objectId, int chunkSize)
 *
 * Load file from local file system at \a filePath and upload it to backend.
 * Define file type with \a contentType (for example \c "image/png" ).
 * \a objectType and \a objectId defines Enginio object which contains reference
 * to uploaded file. If referencing object is not created yet at time of upload,
 * \a objectId can be empty string.
 *
 * For upload file is divided to chunks which are uploaded separately. Optional
 * \a chunkSize argument can be used define size of these chunks.
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
