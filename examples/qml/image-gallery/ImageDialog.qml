import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0


/*
 * Dialog window for displaying image in full size.
 */
Rectangle {
    id: root
    property alias source: image.source
    color: "#646464"
    opacity: visible ? 1.0 : 0.0

    Label {
        id: label
        text: "Loading ..."
        color: "white"
        anchors.centerIn: parent
        visible: image.status != Image.Ready
    }
    ProgressBar {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: label.bottom
        anchors.topMargin: 10
        height: 20
        width: root.width / 3
        minimumValue: 0.0
        maximumValue: 1.0
        value: image.progress
        visible: image.status != Image.Ready
    }

    Image {
        id: image
        anchors.fill: parent
        anchors.margins: 10
        smooth: true
        fillMode: Image.PreserveAspectFit
    }
    Behavior on opacity {
        NumberAnimation {
            duration: 300
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.visible = false
    }
}
