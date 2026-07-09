import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

ColumnLayout {
    id: root

    property int outputRouting: 0
    property bool isLocked: false
    property int slotIndex: -1
    property var slotModel: null

    Layout.fillWidth: true
    spacing: 4

    readonly property var routingList: [
        { icon: "headset",    label: "Broadcast & Monitor", modeColor: "#BB86FC" },
        { icon: "mic",        label: "Broadcast only",      modeColor: "#03DAC6" },
        { icon: "headphones", label: "Monitor only",        modeColor: "#4caf50" }
    ]

    SectionLabel {
        text: "OUTPUT ROUTING"
    }

    Rectangle {
        id: trigger
        Layout.fillWidth: true
        implicitHeight: 34
        enabled: !root.isLocked
        color: triggerMouse.containsMouse && enabled ? "#1a1a1a" : Theme.inputBackground
        border.color: triggerMouse.containsMouse && enabled ? Theme.borderHover : Theme.borderDefault
        border.width: 1
        radius: 8
        opacity: enabled ? 1.0 : 0.6

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
        Behavior on border.color {
            ColorAnimation { duration: 150 }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 8
            spacing: 8

            Image {
                source: "image://icons/" + root.routingList[root.outputRouting].icon
                        + "?color=%23" + root.routingList[root.outputRouting].modeColor.replace("#", "")
                sourceSize: Qt.size(14, 14)
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                text: root.routingList[root.outputRouting].label
                color: enabled ? Theme.textPrimary : Theme.textDim
                font.pixelSize: 12
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            Image {
                source: "image://icons/chevron-down?color=%23" + (triggerMouse.containsMouse ? "b0b0b0" : "888888")
                sourceSize: Qt.size(8, 8)
                Layout.alignment: Qt.AlignVCenter
                visible: enabled
            }
        }

        MouseArea {
            id: triggerMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
            onClicked: {
                if (enabled)
                    routingMenu.openRelativeTo(trigger, root)
            }
        }
    }

    SaikoIconMenu {
        id: routingMenu
        model: root.routingList
        currentIndex: root.outputRouting
        onActivated: function(index) {
            if (root.slotIndex >= 0)
                root.slotModel.setRouting(root.slotIndex, index)
        }
    }
}
