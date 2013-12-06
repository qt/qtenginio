/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtEnginio module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.1
import QtQuick.Controls 1.0
import Enginio 1.0

Rectangle {
    property string listId
    property string listName
    property var aclData

    property var userNames: new Array
    property var userData: new Object

    Component.onCompleted: {
        var aclQuery = enginioClient.query({ "objectType": "objects.todoLists", "id": listId }, Enginio.AccessControlOperation)
        aclQuery.finished.connect(function() { aclData = aclQuery.data } )

        var usersQuery = enginioClient.query({ "objectType": "users", }, Enginio.UsersOperation)
        usersQuery.finished.connect(function() {
            var userNamesTmp = new Array
            for (var i = 0; i < usersQuery.data.results.length; ++i) {
                var user = usersQuery.data.results[i]
                userNamesTmp.push(user["username"])
                userData[user["username"]] = user
            }

            userNames = userNamesTmp
        } )
    }

    Header {
        id: header
        text: "Share " + listName
    }

    ListView {
        id: nameView
        model: userNames
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        delegate: BorderImage {
            id: item

            width: parent.width ; height: 40 * scaleFactor
            source: mouse.pressed ? "qrc:images/delegate_pressed.png" : "qrc:images/delegate.png"
            border.left: 5; border.top: 5
            border.right: 5; border.bottom: 5

            Image {
                id: shadow
                anchors.top: parent.bottom
                width: parent.width
                visible: !mouse.pressed
                source: "qrc:images/shadow.png"
            }

            Item {
                id: checkBox
                height: 40 * scaleFactor
                width: 42 * scaleFactor
                Image {
                    anchors.centerIn: parent
                    fillMode: Image.PreserveAspectFit
                    source: sharedWithUser(userData[modelData].id) ? "qrc:images/checkmark.png" : ""
                }
            }
            Text {
                anchors.left: checkBox.right
                anchors.verticalCenter: parent.verticalCenter
                text: modelData
                font.pixelSize: 22 * scaleFactor
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var id = userData[modelData].id
                    aclData["read"].push({ "id": id, "objectType": "users" })
                    aclData["admin"].push({ "id": id, "objectType": "users" })
                    console.log("\nUpdated JSON:", JSON.stringify(aclData))
                    enginioClient.update(
                                { "id": listId, "objectType": "objects.todoLists", "access": aclData },
                                Enginio.AccessControlOperation
                                )

                    // changing the contents of a JSON object does not emit
                    // the update signal for performance reasons
                    aclDataChanged()
                }
            }
        }
    }

    function sharedWithUser(userId) {
        for (var i = 0; i < aclData["read"].length; ++i)
            if (aclData["read"][i]["id"] === userId)
                return true;
        return false;
    }
}
