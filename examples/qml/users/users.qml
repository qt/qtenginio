import QtQuick 2.0
import Enginio 1.0
import "../config.js" as AppConfig

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
        operation: Enginio.UserOperation
    }

    Component {
        id: ldelegate
        Rectangle {
            width: lview.width
            height: 80
            color: Qt.lighter("lightgray")
            Column {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 1
                Text { text: "User: " + firstName + " " + lastName; font.bold: true }
                Text { text: " Login: " + username }
                Text { text: " Created: " + createdAt }
                Text { text: " Last updated: " + updatedAt }
            }
        }
    }

    Column {
        anchors.margins: 5
        anchors.fill: parent
        spacing: 10
        TextInput {
            id: tifilter
            width: parent.width
            height: 30
            text: "filter example: {\"username\": \"john\" }"
            onAccepted: {
                try {
                    var o = JSON.parse(text)
                    emodel.query = {"query": o}
                } catch(err) {
                    tifilter.text = "invalid json was used for filtering"
                    emodel.query = {}
                }
            }
        }

        ListView {
            id: lview
            width: parent.width
            height: parent.height - parent.spacing - tifilter.height
            model: emodel
            delegate: ldelegate
            spacing: 5
        }
    }
}
