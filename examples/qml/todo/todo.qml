import QtQuick 2.0
import Enginio 1.0
import "qrc:///config.js" as AppConfig

Rectangle {
    id: root
    width: 400; height: 800
    color: "lightgray"

    EnginioModel
    {
        id: emodel
        enginio: Enginio
                 {
                     backendId: AppConfig.backendData.id
                     backendSecret: AppConfig.backendData.secret
                 }
        query: {"objectType": "objects.todo" }
    }

    Component {
        id: ldelegate
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
                        emodel.setProperty(index, "completed", !completed)
                    }
                }
            }
            TextInput {
                id: textTitle
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: title
                color: model.completed ? "green" : "red"
                font.pointSize: 23
                onAccepted: {
                     if (index !== -1) {
                        emodel.setProperty(index, "title", text)
                    }
                }
            }
            Rectangle {
                id: sync
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
                color: textTitle.color
            }

            Rectangle {
                id: rem
                width: 25
                height: 6

                anchors.margins: 5
                anchors.top: parent.top
                anchors.right: parent.right
                enabled: _synced
                opacity: 0
                color: "red"
                states: State {
                    name: "visible"
                    when: mouse.containsMouse
                    PropertyChanges { target: rem; opacity: 1}
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log(index)
                        if (index !== -1)
                            emodel.remove(index)
                    }
                }
            }
        }
    }

    ListView {
        id: lview
        anchors.margins: 5
        anchors.fill: parent

        model: emodel
        delegate: ldelegate
        spacing: 20
    }

    Rectangle {
        id: irec
        opacity: 0.9
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 80
        TextInput {
            id: tinput
            anchors.left: parent.left
            anchors.horizontalCenter: parent.horizontalCenter

            text: "New todo..."
            font.pointSize: 23
            onAccepted: {
                emodel.append({"title": text, "completed": false})
                text = "New todo..."
            }
        }
    }
}
