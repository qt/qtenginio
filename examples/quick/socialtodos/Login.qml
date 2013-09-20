/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Quick Controls module of the Qt Toolkit.
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
import QtQuick.Layouts 1.0
import Enginio 1.0

Rectangle {
    width: 400
    height: 600

    Rectangle {
        id: header
        anchors.top: parent.top
        width: parent.width
        height: 70
        color: "white"

        Row {
            id: logo
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: -4
            spacing: 4
            Image {
                source: "qrc:images/enginio.png"
                width: 160 ; height: 60
                fillMode: Image.PreserveAspectFit
            }
            Text {
                text: "Todos"
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: -3
                font.bold: true
                font.pixelSize: 46
                color: "#555"
            }
        }
        Rectangle {
            width: parent.width ; height: 1
            anchors.bottom: parent.bottom
            color: "#bbb"
        }
    }

    BorderImage {
        id: input

        width: parent.width
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        source: "qrc:images/delegate.png"
        border.left: 5; border.top: 5
        border.right: 5; border.bottom: 5

        Rectangle {
            y: -1 ; height: 1
            width: parent.width
            color: "#bbb"
        }
        Rectangle {
            y: 0 ; height: 1
            width: parent.width
            color: "white"
        }

        ColumnLayout {
            x: 10
            y: 10
            height: parent.height - 2
            width: parent.width - 20
            spacing: 10

            BorderImage {
                Layout.fillWidth: true
                source: "images/textfield.png"
                border.left: 14 ; border.right: 14 ; border.top: 8 ; border.bottom: 8

                TextInput {
                    id: nameInput
                    anchors.fill: parent
                    clip: true
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 22
                    focus: true
                    activeFocusOnTab: true

                    Text {
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        visible: !(parent.text.length)
                        font: parent.font
                        text: "Username"
                        color: "#aaa"
                    }
                    onAccepted: passwordInput.forceActiveFocus()
                }
            }

            BorderImage {
                Layout.fillWidth: true

                height: 40
                source: "images/textfield.png"
                border.left: 14 ; border.right: 14 ; border.top: 8 ; border.bottom: 8

                TextInput{
                    id: passwordInput
                    anchors.fill: parent
                    clip: true
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 22
                    echoMode: TextInput.Password

                    activeFocusOnTab: true
                    Text {
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        visible: !(parent.text.length)
                        font: parent.font
                        text: "Password"
                        color: "#aaa"
                    }
                    onAccepted: login()
                }
            }

            RowLayout {
                // button
                width: parent.width
                height: 60
                spacing: 10

                Button {
                    text: "Login"
                    onClicked: login()
                    enabled: enginioClient.authenticationState !== Enginio.Authenticating && nameInput.text.length && passwordInput.text.length
                }
                Button {
                    text: "Register"
                    onClicked: register()
                    enabled: enginioClient.authenticationState !== Enginio.Authenticating && nameInput.text.length && passwordInput.text.length
                }

            }

            Item {
                Layout.fillHeight: true
            }
            Item {
                width: parent.width
                height: 40
                Text {
                    id: statusText
                }
            }
        }
    }

    Component.onCompleted:
        enginioClient.sessionAuthenticationError.connect(function(reply){
            if (enginioClient.authenticationState === Enginio.AuthenticationFailure)
                statusText.text = "Authentication failed: " + reply.errorString
            }
            )

    function login() {
        statusText.text = "Logging in..."
        auth.user = nameInput.text
        auth.password = passwordInput.text
        enginioClient.identity = auth
    }

    function register() {
        statusText.text = "Creating user account..."
        var reply = enginioClient.create({ "username": nameInput.text, "password": passwordInput.text }, Enginio.UserOperation)
        reply.finished.connect(function() {
            if (reply.errorType !== EnginioReply.NoError) {
                statusText.text = reply.errorString
            } else {
                statusText.text = "Account Created."
                login()
            }
        })
    }
}

