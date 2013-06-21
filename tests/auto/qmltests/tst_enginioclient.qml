import QtQuick 2.0
import QtTest 1.0
import Enginio 1.0
import "config.js" as AppConfig

Item {
    id: root
    property string __testObjectName: "QML_CLIENT_TEST_" + (new Date()).getTime()

    Enginio {
        id: cleanupEnginio
        backendId: AppConfig.backendData.id
        backendSecret: AppConfig.backendData.secret
        serviceUrl: AppConfig.backendData.serviceUrl

        property int errorCount: 0
        property int finishedCount: 0
        onError: {
            finishedCount += 1
            errorCount += 1
            console.log("\n\n### CLEANUP ERROR")
            console.log(reply.errorString)
            reply.dumpDebugInfo()
            console.log("\n###\n")
        }

        onFinished: {
            finishedCount += 1
        }
    }

    function cleanupDatabase() {
        cleanupEnginio.errorCount = 0
        cleanupEnginio.finishedCount = 0

        var reply = cleanupEnginio.query({ "objectType": "objects." + __testObjectName
                              }, Enginio.ObjectOperation);

        cleanupTest.tryCompare(cleanupEnginio, "finishedCount", 1)
        cleanupTest.verify(!cleanupEnginio.errorCount)

        var results = reply.data.results

        cleanupEnginio.finishedCount = 0
        for (var i = 0; i < results.length; ++i)
        {
            cleanupEnginio.remove({ "objectType": "objects." + __testObjectName,
                             "id" : results[i].id
                           }, Enginio.ObjectOperation);
        }

        cleanupTest.tryCompare(cleanupEnginio, "finishedCount", results.length, 10000)
        cleanupTest.verify(!cleanupEnginio.errorCount)

        finishedSpy.clear()
        errorSpy.clear()
    }

    Enginio {
        id: enginio
        backendId: AppConfig.backendData.id
        backendSecret: AppConfig.backendData.secret
        serviceUrl: AppConfig.backendData.serviceUrl

        property int errorCount: 0
        property int finishedCount: 0

        onError: {
            finishedCount += 1
            errorCount += 1
            console.log("\n\n### ERROR")
            console.log(reply.errorString)
            reply.dumpDebugInfo()
            console.log("\n###\n")
        }

        onFinished: {
            finishedCount += 1
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
        id: cleanupTest
        name: "Database clean-up Dummy Test"
    }

    TestCase {
        name: "EnginioClient: ObjectOperation CRUD"

        function init() {
            finishedSpy.clear()
            errorSpy.clear()
        }

        function test_CRUD() {
            var finished = 0;
            var reply = enginio.create({ "objectType": "objects." + __testObjectName,
                                         "testCase" : "EnginioClient_ObjectOperation",
                                         "testName" : "CREATE",
                                         "count" : 1337,
                                       }, Enginio.ObjectOperation);

            finishedSpy.wait()
            compare(finishedSpy.count, ++finished)
            compare(errorSpy.count, 0)

            var objectId = reply.data.id

            compare(reply.data.testName, "CREATE")
            compare(reply.data.count, 1337)

            reply = enginio.query({ "objectType": "objects." + __testObjectName,
                                    "query" : { "id" : objectId }
                                  }, Enginio.ObjectOperation);

            finishedSpy.wait()
            compare(finishedSpy.count, ++finished)
            compare(errorSpy.count, 0)
            verify(reply.data.results !== undefined)
            compare(reply.data.results.length, 1)
            compare(reply.data.results[0].id, objectId)
            compare(reply.data.results[0].testCase, "EnginioClient_ObjectOperation")
            compare(reply.data.results[0].testName, "CREATE")
            compare(reply.data.results[0].count, 1337)

            reply = enginio.update({ "objectType": "objects." + __testObjectName,
                                     "id" : objectId,
                                     "testCase" : "EnginioClient_ObjectOperation_Update",
                                     "testName" : "UPDATE",
                                  }, Enginio.ObjectOperation);

            finishedSpy.wait()
            compare(finishedSpy.count, ++finished)
            compare(errorSpy.count, 0)
            compare(reply.data.id, objectId)
            compare(reply.data.testCase, "EnginioClient_ObjectOperation_Update")
            compare(reply.data.testName, "UPDATE")
            compare(reply.data.count, 1337)

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
            compare(reply.data.results.length, 0)
        }
    }
}
