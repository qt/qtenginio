import QtQuick 2.0
import Enginio 1.0
import "qrc:///config.js" as AppConfig

Rectangle {
    id: root
    width: 400; height: 400
    color: "lightgray"

    property int total: 1
    property int idle: 0
    property int busy: 0

    Enginio {
        id: enginio

        backendId: AppConfig.backendData.id
        backendSecret: AppConfig.backendData.secret

        onFinished: {
            var data = reply.data.results
            root.total = data.length
            var idle = 0
            var busy = 0
            for (var i = 0; i < data.length; ++i) {
                if (data[i].status === "idle")
                    idle += 1
                else if (data[i].status === "busy")
                    busy += 1
            }
            root.idle = idle
            root.busy = busy
        }

        Component.onCompleted: {
            query({ "objectType": "objects.log"
                  , "sort": [{"sortBy": "createdAt", "direction": "desc"}]
                  , "limit": 50
                 })
        }
    }

    Rectangle {
        height: (parent.height) * (idle / total)
        width: parent.width / 2 - 10

        Behavior on height { NumberAnimation { duration: 1000 } }

        anchors.left: parent.left
        anchors.bottom: parent.bottom
        color: "green"
        Text {
            anchors.centerIn: parent
            text: "Idle"
        }
    }

    Rectangle {
        width: parent.width / 2 - 10
        height: (parent.height) * (busy / total)

        Behavior on height { NumberAnimation { duration: 1000 } }

        color: "red"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        Text {
            anchors.centerIn: parent
            text: "Busy"
        }
    }
}
