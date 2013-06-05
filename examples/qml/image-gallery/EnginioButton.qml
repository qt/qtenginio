import QtQuick 2.0

/*
 * Simple button with custom colors.
 */
Rectangle {
    id: button

    property color buttonColor: "#449534"
    property color onHoverColor: "#54a544"
    property color onPressedColor: "#348524"
    property int margin: 10
    property alias text: buttonLabel.text
    property alias textPixelSize: buttonLabel.font.pixelSize

    signal clicked()

    width: buttonLabel.width + margin * 2
    height: buttonLabel.height + margin * 2
    border.color: "white"
    border.width: 2
    state: "NORMAL"

    Text{
        id: buttonLabel
        color: "white"
        anchors.centerIn: parent
        text: "button label"
    }

    MouseArea{
        id: hitbox
        anchors.fill: parent
        onClicked: parent.clicked()
        hoverEnabled: true
        onEntered: parent.state = "HOVER"
        onExited:  parent.state = "NORMAL"
        onPressed: parent.state = "PRESSED"
        onReleased: parent.state = containsMouse ? "HOVER" : "NORMAL"
    }

    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: button
                color: buttonColor
            }
        },
        State {
            name: "HOVER"
            PropertyChanges {
                target: button
                color: onHoverColor
            }
        },
        State {
            name: "PRESSED"
            PropertyChanges {
                target: button
                color: onPressedColor
                scale: 0.95
            }
        }
    ]
}
