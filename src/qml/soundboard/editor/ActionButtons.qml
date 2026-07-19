import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

RowLayout {
    id: root

    property string slotId: ""
    property bool isLocked: false
    property bool hideDelete: false
    property string filePath: ""
    property bool fileExists: true

    property int playState: 0
    property int playbackMode: 0
    property int queueCount: 0

    readonly property int effectivePlaybackMode: playbackMode === 0 ? Backend.settings.defaultPlaybackMode : playbackMode

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
            },
            {
                label: "Preview",
                icon: "headphones",
                color: "03DAC6",
                accentProp: "accentTeal",
                hoverBg: "",
            },
            {
                label: "Stop",
                icon: "square",
                color: "e35d5d",
                accentProp: "accentRed",
                hoverBg: "",
            },
            {
                label: "Delete",
                icon: "trash-2",
                color: "888888",
                accentProp: "accentRed",
                hoverBg: "#2a1a1a",
            }
        ]

        delegate: Button {
            id: actionBtn
            implicitWidth: 80
            implicitHeight: 76

            readonly property bool isToggleStopPlay: index === 0 && root.effectivePlaybackMode === 2 && root.playState !== 0
            property bool isDelete: index === 3
            property bool isAction: index === 0 || index === 1 || index === 2

            visible: {
                if (root.hideDelete && index === 3) return false;
                if (root.effectivePlaybackMode === 2 && index === 2) return false;
                return true;
            }

            enabled: {
                if (root.isLocked) return false
                if (isAction && (!root.fileExists || root.filePath === "")) return false
                if (index === 0 && root.playState === 2 && root.effectivePlaybackMode !== 2) return false
                return true
            }

            background: Rectangle {
                color: parent.enabled && parent.hovered ? (isDelete ? modelData.hoverBg : Theme.inputBackground) : Theme.recessedBackground
                radius: 8
                border.color: parent.enabled && parent.hovered ? (isToggleStopPlay ? Theme.accentRed : Theme[modelData.accentProp]) : Theme.borderDefault
                border.width: 1
            }

            contentItem: ColumnLayout {
                opacity: parent.enabled ? 1.0 : 0.4
                spacing: 6

                Image {
                    source: {
                        var icn = isToggleStopPlay ? "square" : modelData.icon;
                        var clr = isToggleStopPlay ? "e35d5d" : (isDelete && parent.parent.hovered ? "e35d5d" : modelData.color);
                        return "image://icons/" + icn + "?color=%23" + clr;
                    }
                    smooth: true
                    sourceSize: Qt.size(24, 24)
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: isToggleStopPlay ? "Stop" : modelData.label
                    color: isDelete && parent.parent.hovered ? Theme.accentRed : Theme.textPrimary
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            // Queue count overlay pill badge
            Rectangle {
                visible: index === 0 && root.effectivePlaybackMode === 3 && root.queueCount > 0
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.topMargin: 4
                anchors.rightMargin: 4
                width: 20
                height: 16
                radius: 8
                color: Theme.accentRed
                z: 10

                Text {
                    text: "×" + (1 + root.queueCount)
                    color: "white"
                    font.pixelSize: 10
                    font.weight: Font.Bold
                    anchors.centerIn: parent
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: actionBtn.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                acceptedButtons: Qt.NoButton
                hoverEnabled: actionBtn.enabled
            }
            onClicked: {
                if (index === 0 && root.slotId) {
                    if (isToggleStopPlay)
                        Backend.soundboard.stopPlayer(root.slotId);
                    else
                        Backend.soundboard.playPlayer(root.slotId);
                } else if (index === 1 && root.slotId) {
                    Backend.soundboard.playPlayerPreview(root.slotId);
                } else if (index === 2 && root.slotId) {
                    Backend.soundboard.stopPlayer(root.slotId);
                } else if (index === 3) {
                    root.deleteRequested();
                }
            }
        }
    }
}