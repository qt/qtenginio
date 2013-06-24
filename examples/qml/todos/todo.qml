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
            Layout.fillHeight: true
            Layout.fillWidth: true
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
            id: item
            property color gradientColor: "#f4f4f4"
            color: Qt.lighter(root.color)
            width: parent.width
            height: 80
            gradient: Gradient {
                GradientStop { color: mouse.pressed ? Qt.darker(gradientColor, 1.05) : gradientColor; position: 0 }
                GradientStop { color: mouse.pressed ? Qt.darker(gradientColor, 1.05) : Qt.darker(gradientColor, 1.02); position: 1 }
            }
            Rectangle {
                height: 2
                width: parent.width
                color: "#55ffffff"
                visible: !mouse.pressed
            }
            Rectangle {
                height: 2
                width: parent.width
                color: "#22000000"
                anchors.bottom: parent.bottom
            }

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

            Text {
                id: todoText
                text: title
                font.strikeout: completed
                color: completed ? "#999" : "#333"
                font.pointSize: 20

                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 20
                anchors.rightMargin: 40
                elide: Text.ElideRight
            }

            // Show an indication when syncing with the server is in progress
            Text {
                visible: !_synced
                anchors.margins: 4
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                id: syncText
                text: "Syncing..."
            }

            // Show a delete button when the mouse is over the delegate
            Image {
                id: removeIcon
                width: 25
                height: 25

                source: "qrc:icons/delete_icon.png"

                anchors.margins: 20
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                enabled: _synced

                opacity: mouse.containsMouse ? 1.0 : 0.0
                Behavior on opacity { NumberAnimation { duration: 200 } }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (index !== -1)
                            enginioModel.remove(index)
                    }
                }
            }
        }
    }
}
