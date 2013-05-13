import QtQuick 2.0
import Enginio 1.0
import "qrc:///config.js" as AppConfig

Item {
    id: root

    Enginio
    {
        id: enginio
        backendId: AppConfig.backendData.id
        backendSecret: AppConfig.backendData.secret
        onFinished: sync.color = "green"
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

        anchors {
            centerIn: parent
        }

        color: "yellow"
    }

    Timer {
        interval: 50
        running: true
        repeat: true

        onTriggered: {
            sync.color = "red"
            var event = { "status": Math.random(1) > 0.5 ? "busy" : "idle",
                          "objectType" : "objects.log" }
            enginio.create(event)
        }
    }
}
