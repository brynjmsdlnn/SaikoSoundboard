import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

RowLayout {
    id: root

    property bool locked: false
    property bool hasSlot: false
    property string slotId: ""

    Layout.fillWidth: true
    spacing: 8

    Text {
        text: "Slot Details"
        color: Theme.textPrimary
        font.pixelSize: 18
        font.weight: Font.Bold
        Layout.fillWidth: true
    }

    // Lock toggle button in header
    Rectangle {
        id: lockToggle
        implicitWidth: 32
        implicitHeight: 28
        radius: 6

        // Only show when there is a slot AND it is currently unlocked
        visible: root.hasSlot && !root.locked

        // Subtle gold aesthetic on hover
        color: lockToggleArea.containsMouse ? "#1a1008" : "transparent"
        border.color: lockToggleArea.containsMouse ? "#40d99a3d" : "transparent"
        border.width: 1

        Behavior on color {
            ColorAnimation {
                duration: 150
            }
        }
        Behavior on border.color {
            ColorAnimation {
                duration: 150
            }
        }

        Image {
            anchors.centerIn: parent
            sourceSize: Qt.size(14, 14)

            // Swaps the icon name ('lock' vs 'unlock') and its color on hover
            source: lockToggleArea.containsMouse ? "image://icons/lock?color=%23d99a3d" : "image://icons/unlock?color=%23888888"

            // Satisfying click "bounce" effect
            scale: lockToggleArea.pressed ? 0.75 : 1.0
            Behavior on scale {
                NumberAnimation {
                    duration: 150
                    easing.type: Easing.OutBack
                }
            }
        }

        // NEW: Integrated SaikoTooltip component
        SaikoTooltip {
            id: customTooltip
            text: "Lock Slot"
            hovered: lockToggleArea.containsMouse
            direction: "top"
        }

        MouseArea {
            id: lockToggleArea
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onClicked: {
                if (root.slotId)
                    Backend.soundboard.setSlotLocked(root.slotId, true);
            }
        }
    }
}
