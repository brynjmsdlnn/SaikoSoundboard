import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

RowLayout {
    id: root

    property bool locked: false
    property bool hasSlot: false
    property string slotId: ""
    property int playbackMode: 0
    property int slotIndex: -1
    property var slotModel: null
    property string slotName: ""

    // Playback mode data
    readonly property var modeList: [
        {
            icon: "sliders-horizontal",
            label: "Default (Global setting)",
            modeColor: "#888880"
        },
        {
            icon: "refresh-cw",
            label: "Restart (Retrigger)",
            modeColor: "#378ADD"
        },
        {
            icon: "toggle-left",
            label: "Toggle Play / Stop",
            modeColor: "#185FA5"
        },
        {
            icon: "list-ordered",
            label: "Queued Replay (Sequential)",
            modeColor: "#0C447C"
        },
        {
            icon: "square-stack",
            label: "Layered Play (Cut All on Stop)",
            modeColor: "#D85A30"
        },
        {
            icon: "audio-lines",
            label: "Layered Play (Let Ring Out)",
            modeColor: "#993C1D"
        }
    ]

    Layout.fillWidth: true
    spacing: 8

    // ── Inline editable slot name ──────────────────────────────────────
    property bool _editing: false
    property bool _showSuccess: false

    Item {
        Layout.fillWidth: true
        implicitHeight: 28
        clip: true

        HoverHandler {
            id: itemHover
        }

        // ── Display: slot name text + Inline Pencil ──────────────────
        RowLayout {
            id: textDisplayRow
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6
            visible: !root._editing

            Item {
                id: textClip
                Layout.fillWidth: true
                implicitHeight: nameText.implicitHeight
                clip: true

                Text {
                    id: nameText
                    text: root.slotName || "Unnamed Slot"
                    color: root.slotName ? Theme.textPrimary : Theme.textDim
                    font.pixelSize: 18
                    font.weight: Font.Bold
                    elide: Text.ElideNone
                    width: implicitWidth

                    SequentialAnimation on x {
                        running: nameText.implicitWidth > textClip.width && !!root.slotName
                        loops: Animation.Infinite

                        PauseAnimation { duration: 1500 }
                        NumberAnimation {
                            from: 0
                            to: textClip.width - nameText.implicitWidth
                            duration: Math.max(2000, (nameText.implicitWidth - textClip.width) * 25)
                            easing.type: Easing.Linear
                        }
                        PauseAnimation { duration: 2000 }
                        PropertyAction { value: 0 }
                        PauseAnimation { duration: 500 }

                        onRunningChanged: if (!running) nameText.x = 0
                    }
                }
            }

            Image {
                id: inlineEditIcon
                source: "image://icons/pencil?color=" + (nameHover.containsMouse ? "%23d99a3d" : encodeURIComponent(Theme.textDim))
                sourceSize: Qt.size(14, 14)
                Layout.alignment: Qt.AlignVCenter
                opacity: (itemHover.hovered && !root.locked) ? 0.7 : 0.0
                visible: opacity > 0.0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 150
                    }
                }
            }
        }

        // Broad Hover / Click Area over the entire layout container
        MouseArea {
            id: nameHover
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: root.locked ? Qt.ArrowCursor : Qt.IBeamCursor
            onClicked: {
                if (!root.locked) {
                    root._editing = true;
                    Qt.callLater(nameField.forceActiveFocus);
                }
            }
        }

        // ── Editing: TextField ─────────────────────────
        TextField {
            id: nameField
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            visible: root._editing
            text: root.slotName
            placeholderText: "Unnamed Slot"
            color: root._showSuccess ? Theme.accentGreen : Theme.textPrimary

            // Mirroring the exact presentation font style
            font.pixelSize: 18
            font.weight: Font.Bold

            selectByMouse: true
            rightPadding: 36
            leftPadding: 4 // Kept identical to text element margin displacement

            property bool isDirty: activeFocus && text !== root.slotName

            background: Item {} // Fully transparent seamless canvas

            Image {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                source: "image://icons/corner-down-left?color=" + encodeURIComponent(Theme.accentPurple)
                sourceSize: Qt.size(16, 16)
                opacity: nameField.isDirty && !root._showSuccess ? 0.8 : 0.0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 150
                    }
                }
            }

            Image {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                source: "image://icons/check?color=" + encodeURIComponent(Theme.accentGreen)
                sourceSize: Qt.size(18, 18)
                opacity: root._showSuccess ? 1.0 : 0.0
            }

            onEditingFinished: {
                if (root.slotId && text.trim() !== root.slotName) {
                    Backend.soundboard.renamePlayer(root.slotId, text.trim());
                    root._showSuccess = true;
                    successTimer.restart();
                } else if (!root._showSuccess) {
                    root._editing = false;
                }
            }
        }

        Timer {
            id: successTimer
            interval: 1500
            onTriggered: {
                root._showSuccess = false;
                root._editing = false;
            }
        }
    }

    // ── Tooltip cycling ────────────────────────────────────────────────────
    property string _tooltipDisplay: "Playback Mode"
    property bool _showingModeName: false

    Timer {
        id: tooltipTimer
        interval: 500
        onTriggered: {
            if (root._showingModeName) {
                root._tooltipDisplay = "Playback Mode";
                root._showingModeName = false;
                tooltipTimer.interval = 500;
            } else {
                var idx = Math.min(root.playbackMode, root.modeList.length - 1);
                root._tooltipDisplay = root.modeList[idx].label;
                root._showingModeName = true;
                tooltipTimer.interval = 2000;
            }
            tooltipTimer.start();
        }
    }

    onLockedChanged: {
        if (root.locked) {
            tooltipTimer.stop();
            root._showingModeName = false;
            root._tooltipDisplay = "Playback Mode";
        }
    }
    // ── End tooltip ─────────────────────────────────────────────────────────

    // Playback mode icon button
    SaikoIconButton {
        id: modeButton
        visible: root.hasSlot
        isActive: !root.locked
        tooltipText: root._tooltipDisplay
        iconSource: {
            var idx = root.playbackMode;
            if (idx === 0)
                idx = Backend.settings.defaultPlaybackMode;
            idx = Math.min(idx, root.modeList.length - 1);
            var c = root.modeList[idx].modeColor.replace("#", "");
            return "image://icons/" + root.modeList[idx].icon + "?color=%23" + c;
        }
        onClicked: modeMenu.openRelativeTo(modeButton, root)

        // Hover-based tooltip cycling
        onContainsMouseChanged: {
            if (modeButton.containsMouse) {
                root._showingModeName = false;
                root._tooltipDisplay = "Playback Mode";
                tooltipTimer.interval = 500;
                tooltipTimer.start();
            } else {
                tooltipTimer.stop();
                root._showingModeName = false;
                root._tooltipDisplay = "Playback Mode";
            }
        }
    }

    // Playback mode popup menu
    SaikoIconMenu {
        id: modeMenu
        model: root.modeList
        currentIndex: root.playbackMode
        onActivated: function (index) {
            if (root.slotIndex >= 0)
                root.slotModel.setPlaybackMode(root.slotIndex, index);
        }
    }

    // Lock toggle button in header
    SaikoIconButton {
        id: lockToggle
        visible: root.hasSlot && !root.locked
        isActive: true
        tooltipText: "Lock Slot"
        iconSource: "image://icons/unlock?color=%23888888"
        hoveredIconSource: "image://icons/lock?color=%23d99a3d"
        hoverColor: "#1a1008"
        hoverBorderColor: "#40d99a3d"
        onClicked: {
            if (root.slotId)
                Backend.soundboard.setSlotLocked(root.slotId, true);
        }
    }
}
