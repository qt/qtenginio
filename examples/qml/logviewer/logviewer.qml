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

                     onFinished: { busyIndicator.visible = false }
                 }
        query: { "objectType": "objects.log"
               , "sort" : [ { "sortBy": "status", "direction": "asc"} ]
               , "pageSize": 32
               }
    }

    Component {
        id: ldelegate
        Rectangle {
            width: lview.width
            height: 80
            color: Qt.lighter("lightgray")
            Text { text: "Status: " + status + " (" + createdAt + ")"}
        }
    }

    ListView {
        id: lview
        anchors.fill: parent
        anchors.margins: 3
        model: emodel
        delegate: ldelegate
        spacing: 5
    }

    Rectangle {
        id: busyIndicator
        z: 3
        width: 30
        height: 30
        anchors.centerIn: parent

        color: "black"
    }
}
