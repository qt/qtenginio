import QtQuick 2.0
import Enginio 1.0
import "../config.js" as AppConfig

Rectangle {
    id: root
    width: 400; height: 800
    color: "lightgray"

    Enginio
    {
        backendId: AppConfig.backendData.id
        backendSecret: AppConfig.backendData.secret
        property var requests: new Object

        Component.onCompleted: {
            requests.oldest = query({ "objectType": "objects.todos"
                    , "sort" : [ { "sortBy": "createdAt", "direction": "asc"} ]
                    , "limit": 1
                    })
            requests.newest = query({ "objectType": "objects.todos"
                    , "sort" : [ { "sortBy": "createdAt", "direction": "desc"} ]
                    , "limit": 1
                    })

            query({ "objectType": "objects.todos"
                    , "sort" : [ { "sortBy": "createdAt", "direction": "desc"} ]
                    , "limit": 1
                    }).finished.connect(function() { tmessage.text = "I can connect directly to query object too" } )
        }

        onFinished: {
            console.log(reply)
            var data = reply.toObject().results[0];
            switch (reply) {
                case requests.newest: tnewest.text = data.title + " (" + data.createdAt +")" ; break
                case requests.oldest: toldest.text = data.title + " (" + data.createdAt +")" ; break
            }
        }
    }

    Column {
        anchors.centerIn: parent
        Text {
            id: tmessage
        }
        Text {
            id: tnewest
        }
        Text {
            id: toldest
        }
    }
}
