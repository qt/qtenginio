/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
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
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtTest 1.0
import Enginio 1.0
import "config.js" as AppConfig

Item {
    id: root

    Enginio {
        id: enginioClient
        // FIXME Tests depend on the enginioClient being fully initialized
        // currently there is no easy way to enforce it. By initializing
        // serviceUrl first (and exploiting an undefined behavior) we expect that
        // all models will not trigger any query using the default url.
        serviceUrl: AppConfig.backendData.serviceUrl

        property int errorCount: 0
        onError: {
            ++errorCount
            console.log("\n\n### ERROR")
            console.log(reply.errorString)
            reply.dumpDebugInfo()
            console.log("\n###\n")
        }

        backendId: AppConfig.backendData.id
        backendSecret: AppConfig.backendData.secret
    }

    TestCase {
        name: "EnginioModel: create"

        EnginioModel {
            id: modelCreate
        }

        SignalSpy {
            id: modelCreateEnginioChanged
            target: modelCreate
            signalName: "enginioChanged"
        }

        SignalSpy {
            id: modelCreateQueryChanged
            target: modelCreate
            signalName: "queryChanged"
        }

        SignalSpy {
            id: modelCreateOperationChanged
            target: modelCreate
            signalName: "operationChanged"
        }

        function init() {
            modelCreate.enginio = null
        }

        function cleanupTestCase() {
            modelCreate.enginio = null
        }

        function test_assignClient() {
            var signalCount = modelCreateEnginioChanged.count
            modelCreate.enginio = enginioClient
            verify(modelCreate.enginio == enginioClient)
            tryCompare(modelCreateEnginioChanged, "count", ++signalCount)

            modelCreate.enginio = null
            verify(modelCreate.enginio === null)
            tryCompare(modelCreateEnginioChanged, "count", ++signalCount)

            modelCreate.enginio = enginioClient
            verify(modelCreate.enginio == enginioClient)
            tryCompare(modelCreateEnginioChanged, "count", ++signalCount)
        }

        function test_assignQuery() {
            var signalCount = modelCreateQueryChanged.count
            var query = {"objectType": "objects.todos"}
            var queryStr = JSON.stringify(query)
            modelCreate.query = query
            verify(JSON.stringify(modelCreate.query) == queryStr)
            tryCompare(modelCreateQueryChanged, "count", ++signalCount)

            modelCreate.query = {}
            verify(JSON.stringify(modelCreate.query) == "{}")
            tryCompare(modelCreateQueryChanged, "count", ++signalCount)

            modelCreate.query = query
            verify(JSON.stringify(modelCreate.query) == queryStr)
            tryCompare(modelCreateQueryChanged, "count", ++signalCount)
        }

        function test_assignOperation() {
            modelCreate.operation = Enginio.ObjectOperation
            verify(modelCreate.operation === Enginio.ObjectOperation)

            modelCreate.operation = Enginio.ObjectAclOperation
            verify(modelCreate.operation === Enginio.ObjectAclOperation)
        }
    }

    TestCase {
        name: "EnginioModel: query"

        EnginioModel {
            id: modelQuery
            enginio: enginioClient
            onModelAboutToBeReset: { resetCount++ }
            property int resetCount: 0
        }

        function _query(query, operation) {
            var enginioClient = modelQuery.enginio
            modelQuery.enginio = null
            var count = modelQuery.resetCount
            modelQuery.operation = operation
            modelQuery.query = query
            modelQuery.enginio = enginioClient
            tryCompare(modelQuery, "resetCount", ++count, 10000)
        }

        function test_queryObjects() {
            var counterObject = { "counter" : 0, "enginioErrors" : enginioClient.errorCount}
            enginioClient.create({ "objectType": AppConfig.testObjectType,
                               "testCase": "EnginioModel: query",
                               "title": "prepare",
                               "count": 1,
                           }, Enginio.ObjectOperation).finished.connect(function(){ counterObject.counter++});
            enginioClient.create({ "objectType": AppConfig.testObjectType,
                               "testCase": "EnginioModel: query",
                               "title": "prepare",
                               "count": 2,
                           }, Enginio.ObjectOperation).finished.connect(function(){ counterObject.counter++});

            tryCompare(counterObject, "counter", 2, 10000)

            _query({ "limit": 2, "objectType": AppConfig.testObjectType }, Enginio.ObjectOperation)
            compare(counterObject.enginioErrors, enginioClient.errorCount)

        }

        function test_queryUsers() {
            _query({ "limit": 2, "objectType": "users" }, Enginio.UserOperation)
        }

        function test_queryUsersgroups() {
            _query({ "limit": 2, "objectType": "usersgroups" }, Enginio.UsergroupOperation)
        }
    }

    TestCase {
        name: "EnginioModel: modify"

        EnginioModel {
            id: modelModify
            enginio: enginioClient
            query: {
                     "objectType": AppConfig.testObjectType,
                     "query": {"testCase": "EnginioModel: modify"}
                   }

            property int resetCounter: 0
            onModelReset: ++resetCounter
            Component.onCompleted: console.log("start " + modelModify)
            Component.onDestruction: console.log("stop " + modelModify)
        }

        function test_modify() {
            var errorCount = enginioClient.errorCount
            var counterObject = {"counter": 0, "expectedCount": 0}
            tryCompare(modelModify, "resetCounter", 1)

            // append new data
            modelModify.append({ "objectType": AppConfig.testObjectType,
                             "testCase": "EnginioModel: modify",
                             "title": "test_modify",
                             "count": 42,
                         }).finished.connect(function() {counterObject.counter++})
            ++counterObject.expectedCount
            modelModify.append({ "objectType": AppConfig.testObjectType,
                             "testCase": "EnginioModel: modify",
                             "title": "test_modify",
                             "count": 43,
                         }).finished.connect(function() {counterObject.counter++})
            ++counterObject.expectedCount
            tryCompare(counterObject, "counter", counterObject.expectedCount)
            compare(enginioClient.errorCount, errorCount)


            // remove data
            modelModify.remove(0).finished.connect(function(reply) {counterObject.counter++})
            tryCompare(counterObject, "counter", ++counterObject.expectedCount, 10000)
            compare(enginioClient.errorCount, errorCount)


            // change data
            modelModify.setProperty(0, "count", 77).finished.connect(function() {counterObject.counter++})
            tryCompare(counterObject, "counter", ++counterObject.expectedCount)
            compare(enginioClient.errorCount, errorCount)
        }
    }

    TestCase {
        name: "EnginioModel: modify unblocked, wait for the initial reset"

        EnginioModel {
            id: modelModifyUndblocked
            enginio: enginioClient
            query: {
                     "objectType": AppConfig.testObjectType,
                     "query": {"testCase": "EnginioModel: modify unblocked"}
                   }

            property int resetCounter: 0
            onModelReset: ++resetCounter
        }

        function test_modify() {
            var errorCount = enginioClient.errorCount
            var counterObject = {"counter": 0, "expectedCount": 0}
            tryCompare(modelModify, "resetCounter", 1)

            // append new data
            modelModifyUndblocked.append({ "objectType": AppConfig.testObjectType,
                             "testCase": "EnginioModel: modify unblocked",
                             "title": "test_modify",
                             "count": 42,
                         }).finished.connect(function() {counterObject.counter++})
            ++counterObject.expectedCount
            modelModifyUndblocked.append({ "objectType": AppConfig.testObjectType,
                             "testCase": "EnginioModel: modify unblocked",
                             "title": "test_modify",
                             "count": 43,
                         }).finished.connect(function() {counterObject.counter++})
            ++counterObject.expectedCount


            // remove data
            modelModifyUndblocked.remove(1).finished.connect(function(reply) {counterObject.counter++})
            ++counterObject.expectedCount


            // change data
            modelModifyUndblocked.setProperty(0, "count", 77).finished.connect(function() {counterObject.counter++})
            ++counterObject.expectedCount
            tryCompare(counterObject, "counter", counterObject.expectedCount)
            compare(enginioClient.errorCount, errorCount)
        }
    }


    TestCase {
        name: "EnginioModel: modify unblocked chaos"

        EnginioModel {
            id: modelModifyChaos
            enginio: enginioClient
            query: {
                     "objectType": AppConfig.testObjectType,
                     "query": {"testCase": "EnginioModel: modify unblocked chaos"}
                   }

            property int resetCounter: 0
            onModelReset: ++resetCounter
        }

        function test_modify() {
            if (enginioClient.serviceUrl !== "https://staging.engin.io")
                skip("FIXME the test crashes on staging because of enabled notifications")
            var errorCount = enginioClient.errorCount
            var counterObject = {"counter": 0, "expectedCount": 0}

            // append new data
            modelModifyChaos.append({ "objectType": AppConfig.testObjectType,
                             "testCase": "EnginioModel: modify unblocked chaos",
                             "title": "test_modify",
                             "count": 42,
                         }).finished.connect(function() {counterObject.counter++})
            ++counterObject.expectedCount
            modelModifyChaos.append({ "objectType": AppConfig.testObjectType,
                             "testCase": "EnginioModel: modify unblocked chaos",
                             "title": "test_modify",
                             "count": 43,
                         }).finished.connect(function() {counterObject.counter++})
            ++counterObject.expectedCount


            // remove data
            modelModifyChaos.remove(1).finished.connect(function(reply) {counterObject.counter++})
            ++counterObject.expectedCount


            // change data
            modelModifyChaos.setProperty(0, "count", 77).finished.connect(function() {counterObject.counter++})
            ++counterObject.expectedCount
            tryCompare(counterObject, "counter", counterObject.expectedCount)
            compare(enginioClient.errorCount, errorCount)
        }
    }

}
