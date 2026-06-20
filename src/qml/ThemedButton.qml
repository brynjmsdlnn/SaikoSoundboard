import QtQuick
import Saiko 1.0

Rectangle {
    id: btn
    property string text: ""
    property string iconText: ""
    property color accentColor: Theme.accentPurple
    property bool filled: false
    property bool small: false

    signal clicked()

    implicitWidth: small ? 70 : 100
    implicitHeight: small ? 28 : 34
    radius: Theme.borderRadius
    color: filled ? (hovered ? Qt.lighter(accentColor, 1.2) : accentColor)
                  : (hovered ? "#1a1a1a" : Theme.inputBackground)
    border.color: hovered ? accentColor : Theme.borderDefault
    border.width: 1
    opacity: enabled ? 1.0 : 0.4

    property bool hovered: false

    Behavior on color { ColorAnimation { duration: Theme.animDuration } }
    Behavior on border.color { ColorAnimation { duration: Theme.animDuration } }

    Row {
        anchors.centerIn: parent
        spacing: iconText ? 6 : 0
        Text {
            text: btn.iconText
            color: Theme.textPrimary
            font.pixelSize: 14
            visible: iconText !== ""
        }
        Text {
            text: btn.text
            color: Theme.textPrimary
            font.pixelSize: btn.small ? 11 : 12
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: btn.enabled
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onEntered: btn.hovered = true
        onExited: btn.hovered = false
        onClicked: btn.clicked()
    }
}
