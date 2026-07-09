import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Saiko 1.0

Rectangle {
    id: card
    radius: 8
    color: isSelected ? "#19141f" : (cardHover.hovered ? "#161616" : Theme.cardBackground)
    border.color: {
        if (playState === 1) return Qt.rgba(Theme.accentGreen.r, Theme.accentGreen.g, Theme.accentGreen.b, pulseOpacity);
        if (playState === 2) return Qt.rgba(Theme.accentTeal.r, Theme.accentTeal.g, Theme.accentTeal.b, pulseOpacity);
        if (card.locked) return Theme.warning;
        if (!fileExists && filePath !== "") return Theme.accentRed;
        return isSelected ? Theme.accentPurple : (cardHover.hovered ? Theme.borderHover : Theme.borderDefault);
    }
    border.width: (playState === 1 || playState === 2) ? 2 : (isSelected ? 2 : 1)

    Behavior on color {
        ColorAnimation {
            duration: Theme.animDuration
        }
    }
    Behavior on border.color {
        enabled: playState === 0
        ColorAnimation {
            duration: Theme.animDuration
        }
    }

    property real pulseOpacity: 1.0

    onPlayStateChanged: {
        if (playState === 0) {
            pulseOpacity = 1.0;
        }
    }

    SequentialAnimation on pulseOpacity {
        running: playState !== 0
        loops: Animation.Infinite
        NumberAnimation {
            to: 0.3
            duration: 1000
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            to: 1.0
            duration: 1000
            easing.type: Easing.InOutQuad
        }
    }

    property var slotModel
    property int slotIndex: -1
    property bool isSelected: false
    signal clicked

    property string slotName
    property real durationSec
    property int outputRouting
    property int playbackMode: 0
    property string slotId
    property string filePath
    property bool locked: false
    property bool fileExists: true
    property int startTimeMs: 0
    property int endTimeMs: -1
    property int playState: 0

    property int queueCount: 0
    property var layerPositionsMs: []
    property int currentPositionMs: -1
    readonly property int effectivePlaybackMode: playbackMode === 0 ? Backend.settings.defaultPlaybackMode : playbackMode

    readonly property var _modeInfoList: [
        { icon: "sliders-horizontal", label: "Default (Global setting)",       modeColor: "#888880" },
        { icon: "refresh-cw",         label: "Restart (Retrigger)",            modeColor: "#378ADD" },
        { icon: "toggle-left",        label: "Toggle Play / Stop",             modeColor: "#185FA5" },
        { icon: "list-ordered",       label: "Queued Replay (Sequential)",     modeColor: "#0C447C" },
        { icon: "square-stack",      label: "Layered Play (Cut All on Stop)", modeColor: "#D85A30" },
        { icon: "audio-lines",       label: "Layered Play (Let Ring Out)",    modeColor: "#993C1D" }
    ]
    readonly property var _currentModeInfo: card._modeInfoList[
        Math.min(card.effectivePlaybackMode, card._modeInfoList.length - 1)
    ]

    property var waveformData: null

    HoverHandler {
        id: cardHover
    }

    MouseArea {
        id: clickArea
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: mouse => {
            if (mouse.button === Qt.RightButton) {
                contextMenu.popup();
            } else {
                card.clicked();
            }
        }
    }

    Item {
        z: 50
        anchors.fill: parent
        clip: true
        visible: card.locked

        Rectangle {
            x: parent.width - 20 - width / 2
            y: 20 - height / 2
            width: 70
            height: 14
            color: Theme.warning
            rotation: 45
            transformOrigin: Item.Center

            Text {
                anchors.centerIn: parent
                text: "LOCKED"
                color: "#1a1008"
                font.pixelSize: 8
                font.bold: true
                font.letterSpacing: 1
            }
        }
    }

    // Mode badge — completely independent overlay filling the card width
        Item {
            z: 90
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 8 // Aligns perfectly vertically between the header elements
            visible: !card.locked

            Rectangle {
                id: modeBadge
                property bool hovered: false

                anchors.horizontalCenter: parent.horizontalCenter
                width: hovered ? labelText.implicitWidth + 18 : 22
                height: 22
                radius: 11
                color: "#121212"

                // No border when unhovered; lights up with theme color on hover
                border.color: hovered ? card._currentModeInfo.modeColor : "transparent"
                border.width: hovered ? 1.5 : 0

                Behavior on width { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
                Behavior on border.color { ColorAnimation { duration: 150 } }

                // --- SONAR PULSE EFFECT ---
                Rectangle {
                    id: sonarRing
                    anchors.centerIn: parent
                    // Matches base dimensions
                    width: 22
                    height: 22
                    radius: 11
                    // Samples current mode color with soft alpha transparency
                    color: "transparent"
                    border.color: card._currentModeInfo.modeColor
                    border.width: 1
                    opacity: 0.0
                    scale: 1.0
                    z: -1 // Sits cleanly behind the main badge structure
                }

                // --- MASTER ANIMATION TIMELINES ---
                ParallelAnimation {
                    id: idleFxAnimation

                    // 1. The Eye Blink (Squash & Pop)
                    SequentialAnimation {
                        NumberAnimation { target: modeBadge; property: "scale"; to: 0.0; duration: 80; easing.type: Easing.InQuad }
                        NumberAnimation { target: modeBadge; property: "scale"; to: 1.0; duration: 120; easing.type: Easing.OutQuad }
                    }

                    // 2. The Sonar Pulse Ripple
                    SequentialAnimation {
                        // Instant visibility reset
                        PropertyAnimation { target: sonarRing; property: "opacity"; to: 0.6; duration: 0 }
                        PropertyAnimation { target: sonarRing; property: "scale"; to: 1.0; duration: 0 }

                        // Ripple outward and dissipate simultaneously
                        ParallelAnimation {
                            NumberAnimation {
                                id: sonarScaleAnim
                                target: sonarRing; property: "scale"
                                to: 2.3; duration: 600 // Base speed overridden dynamically by timer
                                easing.type: Easing.OutCubic
                            }
                            NumberAnimation {
                                id: sonarOpacityAnim
                                target: sonarRing; property: "opacity"
                                to: 0.0; duration: 600 // Base speed overridden dynamically by timer
                                easing.type: Easing.OutQuad
                            }
                        }
                    }
                }

                // --- RANDOM INTERVAL & SPEED ENGINE ---
                Timer {
                    id: fxTimer
                    running: !modeBadge.hovered // Only run when idle
                    repeat: true
                    triggeredOnStart: false
                    interval: 5000 // Initial delay

                    onTriggered: {
                        // 1. Randomize Pulse Speed (e.g., fast snap ripple vs slow wave between 400ms - 900ms)
                        var randomSpeed = Math.floor(Math.random() * 500) + 400;
                        sonarScaleAnim.duration = randomSpeed;
                        sonarOpacityAnim.duration = randomSpeed;

                        // 2. Fire the synchronized compound effects
                        idleFxAnimation.start();

                        // 3. Randomize next idle rest interval (between 4 to 10 seconds)
                        interval = Math.floor(Math.random() * 6000) + 4000;
                    }
                }

                // --- CONTENT ELEMENTS ---
                Image {
                    anchors.centerIn: parent
                    source: "image://icons/" + card._currentModeInfo.icon + "?color=%23" + card._currentModeInfo.modeColor.replace("#", "")
                    sourceSize: Qt.size(12, 12)
                    scale: modeBadge.hovered ? 0.0 : 1.0
                    opacity: modeBadge.hovered ? 0.0 : 1.0
                    Behavior on scale { NumberAnimation { duration: 120 } }
                    Behavior on opacity { NumberAnimation { duration: 100 } }
                }

                Text {
                    id: labelText
                    anchors.centerIn: parent
                    text: card._currentModeInfo.label
                    color: "#FFFFFF"
                    font.pixelSize: 10
                    font.weight: Font.Medium
                    scale: modeBadge.hovered ? 1.0 : 0.0
                    opacity: modeBadge.hovered ? 1.0 : 0.0
                    Behavior on scale { NumberAnimation { duration: 220; easing.type: Easing.OutBack } }
                    Behavior on opacity { NumberAnimation { duration: 150 } }
                }

                MouseArea {
                    anchors.fill: parent;
                    hoverEnabled: true;
                    cursorShape: Qt.PointingHandCursor;
                    onEntered: {
                        modeBadge.hovered = true
                        idleFxAnimation.stop() // Hard stops current animation loop if triggered mid-way
                        sonarRing.opacity = 0.0 // Clears hanging rings
                    }
                    onExited: {
                        modeBadge.hovered = false
                        modeBadge.scale = 1.0
                    }
                }
            }
        }

    ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 4

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "#" + ("00" + (slotIndex + 1)).slice(-2)
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSizeSmall
                }
                Item { Layout.fillWidth: true }

                RowLayout {
                    visible: !card.fileExists && card.filePath !== ""
                    spacing: 4
                    Image {
                        source: "image://icons/triangle-alert?color=" + encodeURIComponent(Theme.accentRed)
                        sourceSize: Qt.size(12, 12)
                        Layout.alignment: Qt.AlignVCenter
                    }
                    Text {
                        text: "Missing"
                        color: Theme.accentRed
                        font.pixelSize: Theme.fontSizeSmall
                        font.bold: true
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                Text {
                    visible: card.fileExists || card.filePath === ""
                    text: {
                        if (durationSec <= 0) return "0.0s";
                        var totalMs = durationSec * 1000;
                        var startMs = card.startTimeMs;
                        var endMs = card.endTimeMs === -1 ? totalMs : card.endTimeMs;

                        if (card.playState === 0)
                            return ((endMs - startMs) / 1000).toFixed(1) + "s";

                        var pos = card.currentPositionMs;
                        if ((card.effectivePlaybackMode === 4 || card.effectivePlaybackMode === 5) && card.layerPositionsMs.length > 0)
                            pos = card.layerPositionsMs[card.layerPositionsMs.length - 1];

                        if (pos < 0) return ((endMs - startMs) / 1000).toFixed(1) + "s";
                        return (Math.max(0, endMs - pos) / 1000).toFixed(1) + "s";
                    }
                    color: card.locked ? "#555555" : Theme.textDim
                    font.pixelSize: Theme.fontSizeSmall
                }
            }

            Item {
                id: nameClip
                Layout.fillWidth: true
                implicitHeight: nameText.implicitHeight
                clip: true

                Text {
                    id: nameText
                    text: slotName || "Empty Slot"
                    color: card.locked ? "#555555" : (slotName ? Theme.textPrimary : Theme.textDim)
                    font.pixelSize: Theme.fontSizeNormal
                    font.weight: Font.Medium
                    elide: Text.ElideNone
                    width: implicitWidth

                    SequentialAnimation on x {
                        running: card.isSelected && nameText.implicitWidth > nameClip.width && !!slotName
                        loops: Animation.Infinite

                        PauseAnimation { duration: 1500 }
                        NumberAnimation {
                            from: 0
                            to: nameClip.width - nameText.implicitWidth
                            duration: Math.max(2000, (nameText.implicitWidth - nameClip.width) * 25)
                            easing.type: Easing.Linear
                        }
                        PauseAnimation { duration: 2000 }
                        PropertyAction { value: 0 }
                        PauseAnimation { duration: 500 }

                        onRunningChanged: if (!running) nameText.x = 0
                    }
                }
            }

            // mini waveform
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                color: "transparent"
                clip: true

                WaveformView {
                    id: miniWaveform
                    anchors.fill: parent
                    readOnly: true
                    startMs: card.startTimeMs
                    endMs: card.endTimeMs
                    waveformData: card.waveformData
                    filePath: card.filePath
                    fileExists: card.fileExists
                    emptyText: "Empty Slot"
                    layerPositionsMs: card.layerPositionsMs

                    // Almost completely ghost the waveform
                    opacity: card.locked ? 0.15 : 1.0
                }
            }

        RowLayout {
            spacing: 8
            Layout.fillWidth: true

            Rectangle {
                id: playButton
                width: 20
                height: 20
                radius: 4
                color: (!card.fileExists && card.filePath !== "") ? Theme.inputBackground : (playMouse.containsMouse ? (playState !== 0 ? Theme.accentRed : Theme.accentPurple) : Theme.inputBackground)
                border.color: (!card.fileExists && card.filePath !== "") ? Theme.borderDefault : (playMouse.containsMouse ? (playState !== 0 ? Theme.accentRed : Theme.accentPurple) : Theme.borderDefault)
                border.width: 1
                opacity: (!card.fileExists && card.filePath !== "") ? 0.3 : 1.0
                Behavior on color {
                    ColorAnimation {
                        duration: 100
                    }
                }

                Image {
                    anchors.centerIn: parent
                    source: "image://icons/" + (playState !== 0 ? "square" : "play") + "?color=%23" + ((playMouse.containsMouse && (card.fileExists || card.filePath === "")) ? "1e1e1e" : "b0b0b0")
                    width: 10
                    height: 10
                }

                MouseArea {
                    id: playMouse
                    anchors.fill: parent
                    hoverEnabled: card.fileExists || card.filePath === ""
                    cursorShape: (!card.fileExists && card.filePath !== "") ? Qt.ArrowCursor : Qt.PointingHandCursor
                    onClicked: {
                        if (slotId && (card.fileExists || card.filePath === "")) {
                            if (playState !== 0) {
                                Backend.soundboard.stopPlayer(slotId);
                            } else {
                                Backend.soundboard.playPlayer(slotId);
                            }
                        }
                    }
                }
            }

            RowLayout {
                spacing: 4
                Layout.fillWidth: true

                Item { Layout.fillWidth: true }

                Image {
                    source: {
                        var icon = outputRouting === 0 ? "headset" : (outputRouting === 1 ? "mic" : "headphones");
                        var c = outputRouting === 0 ? Theme.accentPurple : (outputRouting === 1 ? Theme.accentTeal : Theme.accentGreen);
                        return "image://icons/" + icon + "?color=" + encodeURIComponent(c);
                    }
                    sourceSize: Qt.size(12, 12)
                    Layout.alignment: Qt.AlignVCenter
                }

                Text {
                    text: outputRouting === 0 ? "Broadcast & Monitor" : (outputRouting === 1 ? "Broadcast only" : "Monitor only")
                    color: {
                        if (outputRouting === 0) return Theme.accentPurple;
                        if (outputRouting === 1) return Theme.accentTeal;
                        return Theme.accentGreen;
                    }
                    font.pixelSize: 9
                    elide: Text.ElideRight
                    Layout.alignment: Qt.AlignVCenter
                }
            }

            Rectangle {
                id: queueBadge
                visible: card.effectivePlaybackMode === 3 && card.queueCount > 0
                Layout.preferredWidth: 20
                Layout.preferredHeight: 16
                radius: 8
                color: Theme.accentRed
                Text {
                    text: "×" + card.queueCount
                    color: "white"
                    font.pixelSize: 10
                    font.weight: Font.Bold
                    anchors.centerIn: parent
                }
            }

        }
    }

    Connections {
        target: Backend.soundboard
        function onWaveformGenerated(playerId, data) {
            if (playerId === slotId && data.isValid) {
                card.waveformData = data;
            }
        }
        function onPlayerPositionChanged(playerId, position) {
            if (playerId === slotId) {
                card.currentPositionMs = position;
                miniWaveform.playPositionMs = position;
            }
        }
        function onPlayerQueueCountChanged(playerId, count) {
            if (playerId === slotId) {
                card.queueCount = count;
            }
        }
        function onPlayerLayerPositionsChanged(playerId, positions) {
            if (playerId === slotId) {
                card.layerPositionsMs = positions;
            }
        }
    }

    onSlotIdChanged: {
        loadWaveform();
        updateQueueAndLayers();
    }

    onFilePathChanged: {
        loadWaveform();
    }

    Component.onCompleted: {
        loadWaveform();
        updateQueueAndLayers();
    }

    function updateQueueAndLayers() {
        card.queueCount = slotId ? Backend.soundboard.getPlayerQueueCount(slotId) : 0;
        card.layerPositionsMs = slotId ? Backend.soundboard.getPlayerLayerPositions(slotId) : [];
    }

    function loadWaveform() {
        if (filePath && filePath !== "") {
            var wfData = Backend.soundboard.getWaveformData(slotId);
            if (wfData && wfData.isValid) {
                card.waveformData = wfData;
            } else {
                Backend.soundboard.loadWaveformData(slotId, filePath);
            }
        } else {
            card.waveformData = null;
        }
    }

    SaikoMenu {
        id: contextMenu
        SaikoMenuItem {
            text: card.locked ? "Unlock Slot" : "Lock Slot"
            onClicked: {
                if (slotId)
                    Backend.soundboard.setSlotLocked(slotId, !card.locked);
            }
        }
        SaikoMenuItem {
            text: "Assign from file..."
            enabled: !card.locked
            onClicked: assignFileDialog.open()
        }
        SaikoMenuItem {
            text: "Assign from replay buffer"
            enabled: !card.locked
            onClicked: {
                if (slotId) {
                    Backend.actions.dispatchAssignReplay(slotId);
                }
            }
        }
        SaikoMenuItem {
            text: "Delete slot"
            enabled: !card.locked
            onClicked: removeConfirmDialog.open()
        }
    }

    FileDialog {
        id: assignFileDialog
        title: "Select audio file"
        nameFilters: ["Audio files (*.wav *.mp3 *.ogg)"]
        onAccepted: {
            if (selectedFile && slotId) {
                Backend.soundboard.assignAudioFile(slotId, selectedFile);
            }
        }
    }

    MessageDialog {
        id: removeConfirmDialog
        title: "Confirm delete"
        text: "Are you sure you want to delete this player?"
        informativeText: (filePath && filePath !== "") ? "It has an assigned audio file." : ""
        buttons: MessageDialog.Yes | MessageDialog.No
        onAccepted: {
            if (slotId) {
                Backend.soundboard.removePlayer(slotId);
            }
        }
    }
}
