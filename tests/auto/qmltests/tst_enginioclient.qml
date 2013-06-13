import QtQuick 2.0
import QtTest 1.0
import Enginio 1.0
import "config.js" as AppConfig

Item {
    id: root
    property string __testObjectName: "QML_TEST_" + (new Date()).getTime()

    function cleanupDatabase() {
        finishedSpy.clear()
        errorSpy.clear()

        var reply = enginio.query({ "objectType": "objects." + __testObjectName
                              }, Enginio.ObjectOperation);

        cleanupTest.tryCompare(finishedSpy, "count", 1)
        cleanupTest.verify(!errorSpy.count)
        finishedSpy.clear()

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

        cleanupTest.tryCompare(finishedSpy, "count", results.length)
        cleanupTest.verify(!errorSpy.count)

        finishedSpy.clear()
        errorSpy.clear()
    }

    Enginio {
        id: enginio
        backendId: AppConfig.backendData.id
        backendSecret: AppConfig.backendData.secret
        apiUrl: AppConfig.backendData.apiUrl

        onError: {
            console.log("\n\n### ERROR")
            console.log(reply.errorString)
            reply.dumpDebugInfo()
            console.log("\n###\n")
        }
    }

    EnginioAuthentication {
        id: validIdentity
        user: "logintest"
        password: "logintest"
    }

    EnginioAuthentication {
        id: invalidIdentity
        user: "INVALID"
        password: "INVALID"
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

    SignalSpy {
        id: sessionAuthenticatedSpy
        target: enginio
        signalName: "sessionAuthenticated"
    }

    SignalSpy {
        id: sessionAuthenticationErrorSpy
        target: enginio
        signalName: "sessionAuthenticationError"
    }

    TestCase {
        id: cleanupTest
        name: "Database clean-up Dummy Test"
    }

    TestCase {
        name: "EnginioClient: Assign an identity"

        function init() {
            enginio.identity = null
            sessionAuthenticatedSpy.clear()
            sessionAuthenticationErrorSpy.clear()
        }

        function cleanupTestCase() {
            init()
        }

        function test_assignValidIdentity() {
            verify(enginio.authenticationState !== Enginio.Authenticated)
            enginio.identity = validIdentity
            sessionAuthenticatedSpy.wait()
            verify(enginio.authenticationState === Enginio.Authenticated)

            // reassign the same
            enginio.identity = null
            tryCompare(enginio, "isAuthenticated", false)
            enginio.identity = validIdentity
            sessionAuthenticatedSpy.wait()
            verify(enginio.authenticationState === Enginio.Authenticated)
        }

        function test_assignInvalidIdentity() {
            verify(enginio.authenticationState !== Enginio.Authenticated)
            enginio.identity = invalidIdentity
            sessionAuthenticationErrorSpy.wait()
            verify(enginio.authenticationState !== Enginio.Authenticated)
        }
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

    TestCase {
        name: "EnginioClient: ObjectOperation Query"

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

            tryCompare(finishedSpy, "count", iterations, 10000)
            compare(errorSpy.count, 0)

            var finished = 0
            var request = [0, 5, 25, 20, 50]
            var expected = [25, 20, 5, 0]

            finishedSpy.clear()

            reply = enginio.query({ "objectType": "objects." + __testObjectName,
                                    "query": { "iteration" : {  "$in": request } },
                                    "sort": [{"sortBy": "iteration", "direction": "desc"}]
                                  }, Enginio.ObjectOperation);

            finishedSpy.wait()
            compare(finishedSpy.count, ++finished)
            compare(errorSpy.count, 0)
            verify(reply.data.results !== undefined)

            var actualCount = reply.data.results.length
            compare(actualCount, expected.length)

            for (var i = 0; i < actualCount; ++i)
            {
                compare(reply.data.results[i].iteration, expected[i])
            }

            cleanupDatabase()
            finishedSpy.clear()

            reply = enginio.query({ "objectType": "objects." + __testObjectName
                                  }, Enginio.ObjectOperation);

            finishedSpy.wait()
            compare(finishedSpy.count, 1)
            compare(errorSpy.count, 0)
            verify(reply.data.results !== undefined)
            compare(reply.data.results.length, 0)
        }
    }
}
