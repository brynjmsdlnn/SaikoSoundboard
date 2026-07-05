import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Saiko 1.0
import "../shared/utils.js" as Utils

Rectangle {
    id: editor

    // ── Appearance ──────────────────────────────────────────────────────────
    color: Theme.cardBackground
    border.color: Theme.borderDefault
    border.width: 1
    implicitWidth: 420

    // ── Public properties ────────────────────────────────────────────────────
    property var slotModel
    property int slotIndex: -1
    property string slotId: ""
    property string slotName: ""
    property string filePath: ""
    property bool isTemporary: false
    property bool locked: false
    property int startTimeMs: 0
    property int playState: 0
    property int endTimeMs: 0
    property real durationSec: 0.0
    property real volume: 1.0
    property int outputRouting: 0
    property string playHotkey: ""
    property string assignHotkey: ""
    property var waveformData: null
    property bool fileExists: true
    property string lastLoadedSlotId: ""
    property string lastLoadedFilePath: ""

    // ── Helpers ───────────────────────────────────────────────────────────────
    readonly property bool hasSlot: slotIndex >= 0 && slotId !== ""
    readonly property bool isTemp: !!(editor && editor.isTemporary)
    readonly property bool isLocked: !!(editor && editor.locked)

    // ── Empty state ───────────────────────────────────────────────────────────
    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width - 64
        visible: !editor.hasSlot
        spacing: 16

        Image {
            source: "image://icons/sliders-horizontal?color=%23b0b0b0"
            width: 48
            height: 48
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            text: "No Slot Selected"
            color: Theme.textPrimary
            font.pixelSize: 16
            font.weight: Font.Bold
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            text: "Select a soundboard slot from the grid to view and edit its details."
            color: Theme.textDim
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }
    }

    // ── Play/Preview Pulsing Overlay Border ──────────────────────────────────
    Rectangle {
        id: playPulseOverlay
        z: 9
        anchors.fill: parent
        anchors.margins: 4
        color: "transparent"
        border.width: 3
        border.color: editor.playState === 1 ? Theme.accentGreen : (editor.playState === 2 ? Theme.accentTeal : "transparent")
        radius: 4
        visible: editor.hasSlot && editor.playState !== 0 && !editor.isLocked
        opacity: pulseOpacity

        property real pulseOpacity: 0.25

        SequentialAnimation on pulseOpacity {
            running: playPulseOverlay.visible
            loops: Animation.Infinite
            NumberAnimation {
                to: 0.65
                duration: 1000
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                to: 0.15
                duration: 1000
                easing.type: Easing.InOutQuad
            }
        }
    }

    // ── Locked overlay ────────────────────────────────────────────────────────
    Rectangle {
        z: 10
        anchors.fill: parent
        visible: editor.hasSlot && editor.isLocked
        color: Qt.rgba(0, 0, 0, 0.95) // Dimmed background

        // Inner glowing frame
        Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            color: "transparent"
            border.color: "#40d99a3d"
            border.width: 3
            radius: 4
        }

        Item {
            anchors.centerIn: parent
            width: 144
            height: 144

            // Curved Text
            Item {
                id: curvedText
                anchors.fill: parent
                z: -1

                property real textRadius: cautionBtnArea.containsMouse ? 92 : 20
                property real textScale: cautionBtnArea.containsMouse ? 1.0 : 0.4
                property real textOpacity: cautionBtnArea.containsMouse ? 1.0 : 0.0

                Behavior on textRadius {
                    NumberAnimation {
                        duration: 450
                        easing.type: Easing.OutBack
                    }
                }
                Behavior on textScale {
                    NumberAnimation {
                        duration: 450
                        easing.type: Easing.OutBack
                    }
                }
                Behavior on textOpacity {
                    NumberAnimation {
                        duration: 250
                    }
                }

                // Dynamic text state (Both are 6 letters, keeping Repeater count stable)
                property string lockText: cautionBtnArea.containsMouse ? "UNLOCK" : "LOCKED"
                property real spreadAngle: 75

                Repeater {
                    model: curvedText.lockText.length
                    Item {
                        anchors.fill: parent

                        property real step: curvedText.spreadAngle / (curvedText.lockText.length - 1)
                        rotation: -(curvedText.spreadAngle / 2) + (index * step)

                        Text {
                            text: curvedText.lockText[index]
                            anchors.horizontalCenter: parent.horizontalCenter
                            y: (parent.height / 2) - curvedText.textRadius - (height / 2)
                            color: Theme.warning
                            font.pixelSize: 22
                            font.bold: true
                            scale: curvedText.textScale
                            opacity: curvedText.textOpacity
                        }
                    }
                }
            }

            // Pulsing Background
            Rectangle {
                id: pulseRing
                anchors.centerIn: parent
                width: parent.width
                height: parent.height
                radius: width / 2
                color: Theme.warning
                visible: cautionBtnArea.containsMouse

                ParallelAnimation {
                    running: cautionBtnArea.containsMouse
                    loops: Animation.Infinite
                    NumberAnimation {
                        target: pulseRing
                        property: "scale"
                        from: 1.0
                        to: 1.4
                        duration: 1000
                        easing.type: Easing.OutQuad
                    }
                    NumberAnimation {
                        target: pulseRing
                        property: "opacity"
                        from: 0.5
                        to: 0.0
                        duration: 1000
                        easing.type: Easing.OutQuad
                    }
                }
            }

            // Main Revert Button
            Rectangle {
                anchors.fill: parent
                radius: width / 2
                color: cautionBtnArea.containsMouse ? Theme.warning : "#1a1008"
                border.color: Theme.warning
                border.width: 2

                Image {
                    anchors.centerIn: parent
                    // CHANGES HERE: Toggles icon source between 'unlock' and 'lock' dynamically
                    source: "image://icons/" + (cautionBtnArea.containsMouse ? "unlock" : "lock") + "?color=" + (cautionBtnArea.containsMouse ? "%231a1008" : encodeURIComponent(Theme.warning))
                    sourceSize: Qt.size(64, 64)
                }

                MouseArea {
                    id: cautionBtnArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: if (editor.slotId)
                        Backend.soundboard.setSlotLocked(editor.slotId, false)
                }
            }
        }

        // Floating action buttons over the lock screen!
        ActionButtons {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 32

            slotId: editor.slotId
            isLocked: false     // Keeps them visually active
            hideDelete: true    // Drops the delete button
            filePath: editor.filePath
            fileExists: editor.fileExists
        }
    }

    // ── Main content ──────────────────────────────────────────────────────────
    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        visible: editor.hasSlot
        topPadding: 20
        bottomPadding: 20
        leftPadding: 20
        rightPadding: 20
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
            width: scroll.availableWidth
            spacing: 20

            EditorHeader {
                locked: editor.isLocked
                hasSlot: editor.hasSlot
                slotId: editor.slotId
            }

            SlotNameEditor {
                slotName: editor.slotName
                slotId: editor.slotId
                isLocked: editor.isLocked
            }

            AudioSourceSelector {
                slotId: editor.slotId
                filePath: editor.filePath
                slotName: editor.slotName
                isTemp: editor.isTemp
                isLocked: editor.isLocked
                fileExists: editor.fileExists
            }

            TrimEditor {
                id: trimEditor
                visible: editor.filePath !== "" && editor.fileExists
                startTimeMs: editor.startTimeMs
                endTimeMs: editor.endTimeMs
                waveformData: editor.waveformData
                isLocked: editor.isLocked
                slotIndex: editor.slotIndex
                slotModel: editor.slotModel
                filePath: editor.filePath
                fileExists: editor.fileExists
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 24

                VolumeSlider {
                    volume: editor.volume
                    isLocked: editor.isLocked
                    slotIndex: editor.slotIndex
                    slotModel: editor.slotModel
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 16

                    RoutingSelector {
                        outputRouting: editor.outputRouting
                        isLocked: editor.isLocked
                        slotIndex: editor.slotIndex
                        slotModel: editor.slotModel
                    }

                    HotkeyDisplay {
                        playHotkey: editor.playHotkey
                        assignHotkey: editor.assignHotkey
                        isLocked: editor.isLocked
                        onRebindRequested: openHotkeyDialog()
                    }

                    ActionButtons {
                        slotId: editor.slotId
                        isLocked: editor.isLocked
                        onDeleteRequested: removeConfirmDialog.open()
                        filePath: editor.filePath
                        fileExists: editor.fileExists
                    }
                }
            }
        }
    }

    // ── Dialogs ───────────────────────────────────────────────────────────────
    MessageDialog {
        id: removeConfirmDialog
        title: "Confirm delete"
        text: "Delete this player?"
        buttons: MessageDialog.Yes | MessageDialog.No
        onAccepted: {
            if (editor.slotId)
                Backend.soundboard.removePlayer(editor.slotId);
        }
    }

    // ── Connections ───────────────────────────────────────────────────────────
    Connections {
        target: SlotModel
        function onDataChanged(topLeft, bottomRight) {
            if (editor.slotIndex >= topLeft.row && editor.slotIndex <= bottomRight.row)
                updateProperties();
        }
        function onModelReset() {
            updateProperties();
        }
    }

    Connections {
        target: Backend.soundboard
        function onWaveformGenerated(playerId, data) {
            if (playerId === editor.slotId && data.isValid)
                editor.waveformData = data;
        }
        function onPlayerPositionChanged(playerId, pos) {
            if (playerId === editor.slotId)
                trimEditor.setPlayPositionMs(pos);
        }
        function onPlayerPlayStateChanged(playerId, state) {
            if (playerId === editor.slotId)
                editor.playState = state;
        }
    }

    // ── Signal handlers ───────────────────────────────────────────────────────
    onSlotIndexChanged: updateProperties()
    onFilePathChanged: loadWaveform()

    // ── Functions ─────────────────────────────────────────────────────────────
    function loadWaveform() {
        if (!editor.filePath || !editor.fileExists) {
            editor.waveformData = null;
            editor.lastLoadedSlotId = "";
            editor.lastLoadedFilePath = "";
            return;
        }
        if (editor.waveformData && editor.lastLoadedSlotId === editor.slotId && editor.lastLoadedFilePath === editor.filePath) {
            return;
        }
        const cached = Backend.soundboard.getWaveformData(editor.slotId);
        if (cached?.isValid) {
            editor.waveformData = cached;
            editor.lastLoadedSlotId = editor.slotId;
            editor.lastLoadedFilePath = editor.filePath;
        } else {
            editor.waveformData = null; // Clear old waveform while loading
            editor.lastLoadedSlotId = "";
            editor.lastLoadedFilePath = "";
            Backend.soundboard.loadWaveformData(editor.slotId, editor.filePath);
        }
    }

    function openHotkeyDialog() {
        Utils.openDialog("../dialogs/HotkeyDialog.qml", {
            slotId: editor.slotId,
            playKey: editor.playHotkey || "",
            assignKey: editor.assignHotkey || ""
        }, function (win) {
            Backend.soundboard.setHotkeys(editor.slotId, win.playKey, win.assignKey);
            win.close();
        });
    }

    function updateProperties() {
        const empty = editor.slotIndex < 0 || editor.slotIndex >= SlotModel.rowCount();
        if (empty) {
            editor.slotId = "";
            editor.slotName = "";
            editor.filePath = "";
            editor.isTemporary = false;
            editor.locked = false;
            editor.fileExists = true;
            editor.startTimeMs = 0;
            editor.endTimeMs = 0;
            editor.durationSec = 0;
            editor.volume = 1;
            editor.outputRouting = 0;
            editor.playHotkey = "";
            editor.assignHotkey = "";
            editor.waveformData = null;
            editor.lastLoadedSlotId = "";
            editor.lastLoadedFilePath = "";
            editor.playState = 0;
            return;
        }
        const d = SlotModel.get(editor.slotIndex);
        editor.slotId = d.slotId ?? "";
        editor.slotName = d.slotName ?? "";
        editor.filePath = d.filePath ?? "";
        editor.isTemporary = d.isTemporary ?? false;
        editor.fileExists = d.fileExists ?? true;
        editor.locked = d.locked ?? false;
        editor.startTimeMs = d.startTimeMs ?? 0;
        editor.endTimeMs = d.endTimeMs ?? 0;
        editor.durationSec = d.durationSec ?? 0;
        editor.volume = d.volume ?? 1;
        editor.outputRouting = d.outputRouting ?? 0;
        editor.playHotkey = d.playHotkey ?? "";
        editor.assignHotkey = d.assignHotkey ?? "";
        editor.playState = d.playState ?? 0;
        loadWaveform();
    }
}
