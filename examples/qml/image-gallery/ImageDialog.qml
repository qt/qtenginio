import QtQuick 2.0
import QtQuick.Window 2.0

/*
 * Dialog window for displaying image in full size.
 */
Window {
    id: root
    property alias source: image.source

    AnimatedImage {
        id: spinner
        anchors.centerIn: parent
        source: "qrc:icons/spinner.gif"
        visible: image.status == Image.Loading
    }

    Image {
        id: image
        anchors.fill: parent
        onStatusChanged: {
            switch (status) {
            case Image.Loading:
                root.width = 200;
                root.height = 100;
                break;
            case Image.Ready:
                root.width = sourceSize.width;
                root.height = sourceSize.height;
                break;
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.visible = false
    }
}
