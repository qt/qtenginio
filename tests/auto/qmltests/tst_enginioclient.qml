import QtQuick 2.0
import QtTest 1.0
import Enginio 1.0
import "config.js" as AppConfig

Item {
    id: root
    property string __testObjectName

    Enginio {
        id: enginio
        backendId: AppConfig.backendData.id
        backendSecret: AppConfig.backendData.secret
        apiUrl: AppConfig.backendData.apiUrl

        onError: {
            console.log(reply.errorString())
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
        name: "EnginioClient_ObjectOperation"

        function initTestCase() {
            __testObjectName = "QML_TEST_" + (new Date()).getTime()
            console.log("The test suffix will be " + __testObjectName)
        }

        function init() {
            finishedSpy.clear()
            errorSpy.clear()
        }

        function test_CRUD() {
            verify(enginio.initialized)
            var finished = 0;
            var reply = enginio.create({ "objectType": "objects." + __testObjectName,
                                         "testCase" : "EnginioClient_ObjectOperation",
                                         "testName" : "CREATE",
                                         "count" : 1337,
                                       }, Enginio.ObjectOperation);

            finishedSpy.wait()
            compare(finishedSpy.count, ++finished)
            compare(errorSpy.count, 0)

            var objectId = reply.data()["id"]

            compare(reply.data()["testName"], "CREATE")
            compare(reply.data()["count"], 1337)

            reply = enginio.query({ "objectType": "objects." + __testObjectName,
                                    "id" : objectId
                                  }, Enginio.ObjectOperation);

            finishedSpy.wait()
            compare(finishedSpy.count, ++finished)
            compare(errorSpy.count, 0)
            compare(reply.data().results.length, 1)
            compare(reply.data().results[0]["id"], objectId)
            compare(reply.data().results[0]["testCase"], "EnginioClient_ObjectOperation")
            compare(reply.data().results[0]["testName"], "CREATE")
            compare(reply.data().results[0]["count"], 1337)

            reply = enginio.update({ "objectType": "objects." + __testObjectName,
                                     "id" : objectId,
                                     "testCase" : "EnginioClient_ObjectOperation_Update",
                                     "testName" : "UPDATE",
                                  }, Enginio.ObjectOperation);

            finishedSpy.wait()
            compare(finishedSpy.count, ++finished)
            compare(errorSpy.count, 0)
            compare(reply.data()["id"], objectId)
            compare(reply.data()["testCase"], "EnginioClient_ObjectOperation_Update")
            compare(reply.data()["testName"], "UPDATE")
            compare(reply.data()["count"], 1337)

            reply = enginio.remove({ "objectType": "objects." + __testObjectName,
                                       "id" : objectId
                                   }, Enginio.ObjectOperation);

            finishedSpy.wait()
            compare(finishedSpy.count, ++finished)
            compare(errorSpy.count, 0)

            reply = enginio.query({ "objectType": "objects." + __testObjectName
                                  }, Enginio.ObjectOperation);

            finishedSpy.wait()
            compare(finishedSpy.count, ++finished)
            compare(errorSpy.count, 0)
            compare(reply.data().results.length, 0)
        }
    }
}
