import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

RowLayout {
    id: root

    property string slotId: ""
    property bool isLocked: false
    property bool hideDelete: false

    signal deleteRequested()

    Layout.alignment: Qt.AlignHCenter
    Layout.topMargin: 4
    spacing: 16

    Repeater {
        model: [
            {
                label: "Play",
                icon: "play",
                color: "4caf50",
                accentProp: "accentGreen",
                hoverBg: "",
                action: () => {
                    if (root.slotId)
                        Backend.soundboard.playPlayer(root.slotId)
                }
            },
            {
                label: "Preview",
                icon: "headphones",
                color: "03DAC6",
                accentProp: "accentTeal",
                hoverBg: "",
                action: () => {
                    if (root.slotId)
                        Backend.soundboard.playPlayerPreview(root.slotId)
                }
            },
            {
                label: "Stop",
                icon: "square",
                color: "e35d5d",
                accentProp: "accentRed",
                hoverBg: "",
                action: () => {
                    if (root.slotId)
                        Backend.soundboard.stopPlayer(root.slotId)
                }
            },
            {
                label: "Delete",
                icon: "trash-2",
                color: "888888",
                accentProp: "accentRed",
                hoverBg: "#2a1a1a",
                action: () => root.deleteRequested()
            }
        ]

        delegate: Button {
            id: actionBtn
            implicitWidth: 80
            implicitHeight: 76

            property bool isDelete: modelData.label === "Delete"

            // Hides the button entirely if the flag is true
            visible: !(root.hideDelete && isDelete)

            enabled: !root.isLocked

            background: Rectangle {
                color: parent.enabled && parent.hovered ? (isDelete ? modelData.hoverBg : Theme.inputBackground) : Theme.recessedBackground
                radius: 8
                border.color: parent.enabled && parent.hovered ? Theme[modelData.accentProp] : Theme.borderDefault
                border.width: 1
            }

            contentItem: ColumnLayout {
                opacity: parent.enabled ? 1.0 : 0.4
                spacing: 6

                Image {
                    source: "image://icons/" + modelData.icon + "?color=%23" + (isDelete && parent.parent.hovered ? "e35d5d" : modelData.color)
                    smooth: true
                    sourceSize: Qt.size(24, 24)
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: modelData.label
                    color: isDelete && parent.parent.hovered ? Theme.accentRed : Theme.textPrimary
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                acceptedButtons: Qt.NoButton
                hoverEnabled: true
            }
            onClicked: modelData.action()
        }
    }
}