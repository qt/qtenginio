import QtQuick 2.0
import QtTest 1.0
import Enginio 1.0
import "config.js" as AppConfig

Item {
    id: root
    property string __testObjectName: "QML_TEST_" + (new Date()).getTime()

    function cleanupDatabase() {
        finishedSpy.clear()

        var reply = enginio.query({ "objectType": "objects." + __testObjectName
                              }, Enginio.ObjectOperation);

        finishedSpy.wait()
        if (typeof reply.data().results == 'undefined') {
            return
        }

        for (var i = 0; i < reply.data().results.length; ++i)
        {
            enginio.remove({ "objectType": "objects." + __testObjectName,
                             "id" : reply.data().results[i]["id"]
                           }, Enginio.ObjectOperation);
        }

        while (finishedSpy.count < reply.data().results.length)
        {
            finishedSpy.wait()
        }

        finishedSpy.clear()
        errorSpy.clear()
    }

    Enginio {
        id: enginio
        backendId: AppConfig.backendData.id
        backendSecret: AppConfig.backendData.secret
        apiUrl: AppConfig.backendData.apiUrl

        onError: {
            console.log("\n\n### ERROR: " + reply.errorString + " ###\n")
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
        name: "EnginioClient: Assign an identity"

        function init() {
            enginio.identity = null
            sessionAuthenticatedSpy.clear()
            sessionAuthenticationErrorSpy.clear()
        }

        function test_assignValidIdentity() {
            verify(!enginio.isAuthenticated)
            enginio.identity = validIdentity
            sessionAuthenticatedSpy.wait()
            verify(enginio.isAuthenticated)

            // reassign the same
            enginio.identity = null
            tryCompare(enginio, "isAuthenticated", false)
            enginio.identity = validIdentity
            sessionAuthenticatedSpy.wait()
            verify(enginio.isAuthenticated)
        }

        function test_assignInvalidIdentity() {
            verify(!enginio.isAuthenticated)
            enginio.identity = invalidIdentity
            sessionAuthenticationErrorSpy.wait()
            verify(!enginio.isAuthenticated)
        }
    }


    TestCase {
        name: "EnginioClient: EnginioClient: ObjectOperation CRUD"

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

            var objectId = reply.data.id

            compare(reply.data.testName, "CREATE")
            compare(reply.data.count, 1337)

            reply = enginio.query({ "objectType": "objects." + __testObjectName,
                                    "id" : objectId
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
