import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0


/*
 * Dialog window for displaying image in full size.
 */
Rectangle {
    id: root
    property alias source: image.source

    color: "white"

    AnimatedImage {
        id: spinner
        anchors.centerIn: parent
        source: "qrc:icons/spinner.gif"
        visible: image.status == Image.Loading

        ProgressBar {
            height: 10
            width: parent.width

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.bottom
            anchors.margins: 10

            maximumValue: 1.0
            minimumValue: 0
            value: image.progress

            Layout.fillWidth: true
        }
    }

    Image {
        id: image
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 300
        }
    }

    state: "hiden"
    states: [
        State {
            name: "shown"
            PropertyChanges {
                target: root
                opacity: 1
                enabled: true
            }
        },
        State {
            name: "hiden"
            PropertyChanges {
                target: root
                opacity: 0
                enabled: false
            }
        }
    ]

    MouseArea {
        anchors.fill: parent
        onClicked: root.state = "hiden"
    }
}
