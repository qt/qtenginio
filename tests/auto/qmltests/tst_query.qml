import QtQuick 2.0
import QtTest 1.0
import Enginio 1.0
import "config.js" as AppConfig

Item {
    id: root
    property string __testObjectName: "QML_QUERY_TEST_" + (new Date()).getTime()

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

    TestCase {
        id: cleanupTest
        name: "Database clean-up Dummy Test"
    }

    TestCase {
        name: "EnginioClient: ObjectOperation Query"

        function initTestCase() {
            cleanupDatabase()
        }

        function cleanupTestCase() {
            cleanupDatabase()
        }

        function init() {
            enginio.errorCount = 0
            enginio.finishedCount = 0
        }

        function test_query() {
            var iterations = 50
            var reply

            for (var i = 0; i < iterations; ++i)
            {
                enginio.create({ "objectType": "objects." + __testObjectName,
                                   "testCase" : "EnginioClient_ObjectOperation",
                                   "testName" : "test_Query_" + i,
                                   "iteration" : i,
                               }, Enginio.ObjectOperation);
            }

            var request = [0, 5, 25, 20, 50]
            var expected = [25, 20, 5, 0]

            tryCompare(enginio, "finishedCount", iterations, 25000)
            compare(enginio.errorCount, 0)
            enginio.finishedCount = 0

            reply = enginio.query({ "objectType": "objects." + __testObjectName,
                                    "query": { "iteration" : {  "$in": request } },
                                    "sort": [{"sortBy": "iteration", "direction": "desc"}]
                                  }, Enginio.ObjectOperation);

            tryCompare(enginio, "finishedCount", 1)
            compare(enginio.errorCount, 0)
            verify(reply.data.results !== undefined)

            var actualCount = reply.data.results.length
            compare(actualCount, expected.length)

            for (var i = 0; i < actualCount; ++i)
            {
                compare(reply.data.results[i].iteration, expected[i])
            }

            cleanupDatabase()
            enginio.finishedCount = 0

            reply = enginio.query({ "objectType": "objects." + __testObjectName
                                  }, Enginio.ObjectOperation);

            tryCompare(enginio, "finishedCount", 1)
            compare(enginio.errorCount, 0)
            verify(reply.data.results !== undefined)
            compare(reply.data.results.length, 0)
        }
    }
}
