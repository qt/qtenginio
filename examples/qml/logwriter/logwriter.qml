import QtQuick 2.0
import Enginio 1.0
import "../config.js" as AppConfig

Item {
    id: root

    Enginio
    {
        id: enginio
        backendId: AppConfig.backendData.id
        backendSecret: AppConfig.backendData.secret
    }

    Timer {
        interval: 50
        running: true
        repeat: true

        onTriggered: {
            var event = { "status": Math.random(1) > 0.5 ? "busy" : "idle",
                          "objectType" : "objects.log" }
            enginio.create(event)
        }
    }
}
