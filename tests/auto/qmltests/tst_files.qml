import QtQuick 2.0
import QtTest 1.0
import Enginio 1.0
import "config.js" as AppConfig


// NOTE this test will only work if the server has the right setup:
// objects.files needs to have fileAttachment set to be a ref to files.


Item {
    id: root
    property string __testObjectName: "QML_TEST_FILES_" + (new Date()).getTime()

    function cleanupDatabase() {
        finishedSpy.clear()

        var reply = enginio.query({ "objectType": "objects." + __testObjectName
                                  }, Enginio.ObjectOperation);

        finishedSpy.wait()

        var results = reply.data.results

        if (results === undefined) {
            console.log("No data to clean up.")
            return
        }

        for (var i = 0; i < results.length; ++i)
        {
            enginio.remove({ "objectType": "objects." + __testObjectName,
                               "id" : results[i].id
                           }, Enginio.ObjectOperation);
        }

        while (finishedSpy.count < results.length)
        {
            finishedSpy.wait() // Throws an exception if it times out
        }

        finishedSpy.clear()
        errorSpy.clear()
    }

    Enginio {
        id: enginio
        backendId: AppConfig.backendData.id
        backendSecret: AppConfig.backendData.secret
        serviceUrl: AppConfig.backendData.serviceUrl

        onError: {
            console.log("\n\n### ERROR")
            console.log(reply.errorString)
            reply.dumpDebugInfo()
            console.log("\n###\n")
        }
    }

    SignalSpy {
        id: finishedSpy
        target: enginio
        signalName: "finished"
    }

    SignalSpy {
        id: errorSpy
        target: enginio
        signalName: "error"
    }

    TestCase {
        name: "Files"

        function initTestCase() {
            cleanupDatabase()
        }

        function cleanupTestCase() {
            cleanupDatabase()
        }

        function init() {
            finishedSpy.clear()
            errorSpy.clear()
        }

        function test_upload() {
            var finished = 0

            //! [upload-create-object]
            var fileObject = {
                "objectType": "objects.files",
                "name": "Example object with file attachment",
            }
            var reply = enginio.create(fileObject);
            //! [upload-create-object]

            finishedSpy.wait()
            compare(finishedSpy.count, ++finished)
            compare(errorSpy.count, 0)
            verify(reply.data.id.length > 0)

            var fileName = AppConfig.testSourcePath + "/../common/enginio.png";

            //! [upload]
            var objectId = reply.data.id
            var uploadData = {
                "file":{
                    "fileName":"test.png"
                },
                "targetFileProperty": {
                    "objectType": "objects.files",
                    "id": objectId,
                    "propertyName": "fileAttachment"
                },
            }
            var uploadReply = enginio.uploadFile(uploadData, fileName)
            //! [upload]

            finishedSpy.wait()
            compare(finishedSpy.count, ++finished)
            compare(errorSpy.count, 0)

            //! [download]
            var downloadData = {
                "id": uploadReply.data.id,
            }
            var downloadReply = enginio.downloadFile(downloadData)
            //! [download]

            finishedSpy.wait()
            compare(finishedSpy.count, ++finished)
            compare(errorSpy.count, 0)

            verify(downloadReply.data.expiringUrl.length > 0)
        }
    }
}
