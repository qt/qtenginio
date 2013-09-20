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
            modelCreate.enginio = enginio
            verify(modelCreate.enginio == enginio)
            tryCompare(modelCreateEnginioChanged, "count", ++signalCount)

            modelCreate.enginio = null
            verify(modelCreate.enginio === null)
            tryCompare(modelCreateEnginioChanged, "count", ++signalCount)

            modelCreate.enginio = enginio
            verify(modelCreate.enginio == enginio)
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
            enginio: enginio
            onModelAboutToBeReset: { resetCount++ }
            property int resetCount: 0
        }

        function test_queryObjects() {
            var counterObject = { "counter" : 0 }
            enginio.create({ "objectType": AppConfig.testObjectType,
                               "testCase": "EnginioModel: query",
                               "title": "prepare",
                               "count": 1,
                           }, Enginio.ObjectOperation).finished.connect(function(){ counterObject.counter++});
            enginio.create({ "objectType": AppConfig.testObjectType,
                               "testCase": "EnginioModel: query",
                               "title": "prepare",
                               "count": 2,
                           }, Enginio.ObjectOperation).finished.connect(function(){ counterObject.counter++});

            tryCompare(counterObject, "counter", 2, 10000)
            var count = modelQuery.resetCount
            modelQuery.query = { "limit": 2, "objectType": AppConfig.testObjectType }
            tryCompare(modelQuery, "resetCount", ++count, 10000)
        }
    }
}
