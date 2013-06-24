import QtQuick 2.0
//![imports]
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import Enginio 1.0
//![imports]
import "qrc:///config.js" as AppConfig

Rectangle {
    id: root
    width: 400; height: 800
    color: "lightgray"

    //![model]
    EnginioModel {
        id: enginioModel
        enginio: Enginio {
            backendId: AppConfig.backendData.id
            backendSecret: AppConfig.backendData.secret
        }
        query: {"objectType": "objects.todo" }
    }
    //![model]


    // A simple layout:
    // a listview and a line edit with button to add to the list

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4

        //![view]
        ListView {
            model: enginioModel
            delegate: listItemDelegate
            spacing: 4
            width: parent.width
            Layout.fillHeight: true
            clip: true
        }
        //![view]

        RowLayout {
            //![append]
            TextField {
                id: textInput
                placeholderText: "Enter ToDo here..."
                onAccepted: {
                    enginioModel.append({"title": text, "completed": false})
                    text = ""
                }
                Layout.fillWidth: true
            }
            //![append]
            Button {
                text: "Add ToDo"
                onClicked: textInput.accepted()
            }
        }
    }

    Component {
        id: listItemDelegate

        Rectangle {
            radius: 10
            color: Qt.lighter(root.color)
            width: parent.width
            height: 80
            MouseArea {
                id: mouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    if (index !== -1) {
                        enginioModel.setProperty(index, "completed", !completed)
                    }
                }
            }

            // Allow editing the title
            TextInput {
                id: todoText
                y: 4
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: title
                color: model.completed ? "green" : "red"
                font.pointSize: 23
                onAccepted: {
                    if (index !== -1) {
                        enginioModel.setProperty(index, "title", text)
                    } else {
                        console.log("whoot?")
                    }
                }
            }

            // Show an indication when syncing with the server is in progress
            Column {
                visible: !_synced
                anchors.margins: 4
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                spacing: 4
                Text {
                    id: syncText
                    text: "Syncing..."
                }
            }

            // Show a delete button when the mouse is over the delegate
            Image {
                id: removeIcon
                width: 25
                height: 25

                source: "qrc:icons/delete_icon.png"

                anchors.margins: 4
                anchors.top: parent.top
                anchors.right: parent.right
                enabled: _synced

                opacity: mouse.containsMouse
                Behavior on opacity { NumberAnimation { duration: 200 } }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log(index)
                        if (index !== -1)
                            enginioModel.remove(index)
                    }
                }
            }

            CheckBox {
                text: "done"
                visible: mouse.containsMouse || hovered
                anchors.margins: 4
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                checked: model.completed
                onClicked: {
                    if (index !== -1) {
                        enginioModel.setProperty(index, "completed", !completed)
                    }
                }
            }
        }
    }
}
