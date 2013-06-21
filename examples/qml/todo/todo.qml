import QtQuick 2.0
import Enginio 1.0
import "qrc:///config.js" as AppConfig

Rectangle {
    id: root
    width: 400; height: 800
    color: "lightgray"

    EnginioModel {
        id: enginioModel
        enginio: Enginio {
                     backendId: AppConfig.backendData.id
                     backendSecret: AppConfig.backendData.secret
                 }
        query: {"objectType": "objects.todo" }
    }

    Component {
        id: listItemDelegate

        Rectangle {
            color: Qt.lighter(root.color)
            width: lview.width
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

            TextInput {
                id: todoText
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: title
                color: model.completed ? "green" : "red"
                font.pointSize: 23
                onAccepted: {
                     if (index !== -1) {
                        enginioModel.setProperty(index, "title", text)
                    }
                }
            }

            Rectangle {
                width: 20
                height: 20
                RotationAnimation on rotation {
                    loops: Animation.Infinite
                    from: 0
                    to: 360
                }

                anchors.top: parent.top
                anchors.left: parent.left
                enabled: false
                visible: !_synced
                color: todoText.color
            }

            Image {
                id: removeIcon
                width: 25
                height: 25

                source: "qrc:icons/delete_icon.png"

                anchors.margins: 5
                anchors.top: parent.top
                anchors.right: parent.right
                enabled: _synced

                opacity: 0
                Behavior on opacity { NumberAnimation { duration: 200 } }

                states: [
                    State {
                        name: "visible"
                        when: mouse.containsMouse
                        PropertyChanges { target: removeIcon; opacity: 1}
                    },
                    State {
                        name: "invisible"
                        when: !mouse.containsMouse
                        PropertyChanges { target: removeIcon; opacity: 0}
                    }
                ]

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log(index)
                        if (index !== -1)
                            enginioModel.remove(index)
                    }
                }
            }
        }
    }

    ListView {
        id: lview
        anchors.margins: 5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: irec.top

        model: enginioModel
        delegate: listItemDelegate
        spacing: 15
    }

    Rectangle {
        id: irec
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 80
        opacity: 0.9

        TextInput {
            id: tinput
            anchors.left: parent.left
            anchors.horizontalCenter: parent.horizontalCenter

            text: "New todo..."
            font.pointSize: 23
            onAccepted: {
                enginioModel.append({"title": text, "completed": false})
                text = "New todo..."
            }
        }
    }
}
