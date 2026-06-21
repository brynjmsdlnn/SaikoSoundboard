import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0
import "utils.js" as Utils

Flickable {
    id: root

    // ---- Public API -------------------------------------------------------

    property string captureMode: "global"
    property string lastRecordingPath: ""

    property bool startEnabled: true
    property bool stopEnabled: false
    property bool playEnabled: false
    property bool modeEnabled: true
    property bool saveReplayEnabled: false
    property bool replayChecked: false
    property string statusText: "Ready"

    signal startRequested()
    signal stopRequested()
    signal changeFolderRequested()
    signal captureModeSelected(string newMode)
    // Emitted instead of mutating root.lastRecordingPath directly. Main.qml
    // binds lastRecordingPath: app.lastRecordingPath one-way; writing to it
    // from in here would fork it from app.lastRecordingPath permanently
    // (QML silently breaks the binding on the first imperative assignment).
    // Main.qml should listen for this and update app.lastRecordingPath itself.
    signal replaySaved(string path)

    function notifyRecordingStarted() {
        startEnabled = false
        stopEnabled = true
        playEnabled = false
        __remainingSec = recordingDurationSec
        __elapsedSec = 0.0
        timerLabel.text = "Time remaining: " + recordingDurationSec + "s"
        statsLabel.text = "Size: 0 KB \u00b7 Time: 0.0s"
        recordingTimer.start()
        stopTimer.start()
    }
    function notifyRecordingStopped() {
        recordingTimer.stop()
        stopTimer.stop()
    }
    function setStatusText(t) { statusText = t }
    function setPlayEnabled(e) { playEnabled = e }
    function setStartEnabled(e) { startEnabled = e }
    function setModeEnabled(e) { modeEnabled = e }
    function setReplayChecked(c) { replayChecked = c }
    function setSaveReplayEnabled(e) { saveReplayEnabled = e }
    function resetUI() {
        timerLabel.text = ""
        statsLabel.text = ""
        stopEnabled = false
        startEnabled = true
    }

    // ---- Layout constants ---------------------------------------------
    // Centralizing these avoids magic numbers scattered through the file
    // and makes future re-theming/resizing a one-line change.

    readonly property int recordingDurationSec: 10
    readonly property int contentMargin: 8
    readonly property int sectionSpacing: 10
    readonly property int cardPadding: 12
    readonly property int headerHeight: 38
    readonly property int waveformHeight: 80
    readonly property int controlsRowHeight: 44
    readonly property int controlButtonWidth: 140
    readonly property int controlButtonHeight: 34
    readonly property int controlButtonSpacing: 16
    readonly property int pulseDurationMs: 800

    // ---- Internal state -----------------------------------------------

    property int __remainingSec: 0
    property real __elapsedSec: 0.0

    SplitView.minimumWidth: 800
    SplitView.minimumHeight: 400
    contentWidth: width
    contentHeight: mainColumn.implicitHeight + 24
    clip: true

    Timer {
        id: recordingTimer
        interval: 100
        repeat: true
        onTriggered: {
            root.__elapsedSec += 0.1
            root.__remainingSec = Math.max(0, root.recordingDurationSec - Math.floor(root.__elapsedSec))
            timerLabel.text = "Time remaining: " + root.__remainingSec + "s"
            var bytes = Backend.recordingFileSize()
            statsLabel.text = "Size: " + Math.round(bytes / 1024) + " KB \u00b7 Time: " + root.__elapsedSec.toFixed(1) + "s"
        }
    }
    Timer {
        id: stopTimer
        interval: recordingDurationSec * 1000
        repeat: false
        onTriggered: root.stopRequested()
    }
    Timer {
        id: playbackTimer
        interval: 100
        repeat: true
        running: false
        onTriggered: {
            var pos = Backend.playbackPosition()
            var dur = Backend.playbackDuration
            timerLabel.text = (pos / 1000).toFixed(1) + "s / " + (dur / 1000).toFixed(1) + "s"
        }
    }
    Connections {
        target: Backend
        function onIsPlayingChanged() {
            if (Backend.isPlaying) {
                playbackTimer.start()
            } else {
                playbackTimer.stop()
                if (timerLabel.text !== "") timerLabel.text = ""
            }
        }
    }

    ColumnLayout {
        id: mainColumn
        width: parent.width - 16
        x: root.contentMargin; y: root.contentMargin
        spacing: root.sectionSpacing

        // ---- HEADER ------------------------------------------------------
        Rectangle {
            Layout.fillWidth: true
            height: root.headerHeight
            color: Theme.appBackground
            radius: Theme.borderRadius
            border.color: Theme.borderDefault

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: root.cardPadding
                anchors.rightMargin: root.cardPadding
                spacing: 8

                Text { text: "((•))"; color: Theme.accentGreen; font.pixelSize: 16; font.bold: true }
                Text { id: statusLabel; text: Backend.isPlaying ? "Playing: " + root.lastRecordingPath.split("/").pop() : root.statusText; color: Theme.textPrimary; font.pixelSize: 14; font.weight: Font.Medium }
                Item { Layout.fillWidth: true }
                Text { id: timerLabel; text: ""; color: Theme.accentPurple; font.pixelSize: 13; visible: text !== "" }
                Text { id: statsLabel; text: ""; color: Theme.textSecondary; font.pixelSize: 13; visible: text !== "" && !Backend.isPlaying; Layout.leftMargin: 8 }
            }
        }

        // ---- CAPTURE MODE + SAVE FOLDER ----------------------------------
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: captureContent.implicitHeight + 24
            color: Theme.appBackground
            radius: Theme.cardRadius
            border.color: Theme.borderDefault

            ColumnLayout {
                id: captureContent
                anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
                anchors.margins: root.cardPadding

                RowLayout {
                    Layout.fillWidth: true; spacing: 10

                    Text { text: "Capture Mode:"; color: Theme.textSecondary }
                    CustomComboBox {
                        id: modeCombo
                        Layout.preferredWidth: 180
                        model: [
                            { text: "System Output (Global)", value: "global" },
                            { text: "Multi-track (sources)", value: "multi" }
                        ]
                        textRole: "text"; valueRole: "value"
                        isActive: root.modeEnabled
                        onActivated: {
                            root.captureMode = currentValue
                            root.captureModeSelected(currentValue)
                        }
                    }

                    Text { text: "Save Folder:"; color: Theme.textSecondary; Layout.leftMargin: 12 }
                    Rectangle {
                        Layout.fillWidth: true; height: 28
                        color: Theme.appBackground; radius: Theme.borderRadius; border.color: Theme.borderDefault
                        TextInput {
                            id: saveDirInput
                            anchors.fill: parent; anchors.margins: 8
                            verticalAlignment: TextInput.AlignVCenter
                            color: Theme.textPrimary
                            readOnly: true
                            text: Backend.settings.saveDirectory
                        }
                    }
                    ThemedButton { text: "Open"; small: true; onClicked: Qt.openUrlExternally("file:///" + Backend.settings.saveDirectory) }
                    ThemedButton { text: "Change"; small: true; onClicked: root.changeFolderRequested() }
                }
            }
        }

        // ---- REPLAY BUFFER -------------------------------------------------
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: replayContent.implicitHeight + 24
            color: Theme.appBackground
            radius: Theme.cardRadius
            border.color: Theme.borderDefault

            ColumnLayout {
                id: replayContent
                anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
                anchors.margins: root.cardPadding
                spacing: 8

                property bool isActive: Backend.recording.isReplayActive

                // -- Title row: status dot + label --
                RowLayout {
                    spacing: 6

                    Rectangle {
                        id: statusDot
                        width: 8; height: 8; radius: 4
                        color: replayContent.isActive ? Theme.accentGreen : Theme.textDim
                        opacity: 1.0

                        SequentialAnimation on opacity {
                            running: replayContent.isActive
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.3; duration: root.pulseDurationMs; easing.type: Easing.InOutQuad }
                            NumberAnimation { to: 1.0; duration: root.pulseDurationMs; easing.type: Easing.InOutQuad }
                        }

                        // Snap back to fully solid as soon as the buffer goes
                        // inactive, instead of freezing mid-fade.
                        Connections {
                            target: replayContent
                            function onIsActiveChanged() {
                                if (!replayContent.isActive) statusDot.opacity = 1.0
                            }
                        }
                    }

                    Text { text: "Replay Buffer"; color: Theme.textPrimary; font.bold: true }
                }

                // -- Controls row: enable checkbox + duration spinner --
                RowLayout {
                    Layout.fillWidth: true

                    CustomCheckBox {
                        id: replayCheck
                        text: "Enable Replay Buffer"
                        checked: root.replayChecked
                        onToggled: {
                            Backend.settings.replayEnabled = checked
                            Backend.settings.save()
                            Backend.recording.setReplayEnabled(checked, root.captureMode)
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Text { text: "Duration:"; color: Theme.textSecondary }
                    Rectangle {
                        width: 70; height: 26
                        color: Theme.appBackground; radius: 4; border.color: Theme.borderDefault

                        HoverHandler { id: hoverHandler }

                        SpinBox {
                            id: replayDurationSpin
                            anchors.fill: parent
                            from: 1; to: 120
                            value: Backend.settings.replayDuration
                            editable: true
                            background: Item {}

                            textFromValue: function(value) { return value + "s" }
                            valueFromText: function(text) {
                                var parsed = parseInt(text.replace("s", ""), 10)
                                return isNaN(parsed) ? replayDurationSpin.value : parsed
                            }

                            down.indicator: Item {
                                x: 0; y: 0; width: 20; height: parent.height
                                opacity: hoverHandler.hovered ? 1.0 : 0.0
                                Behavior on opacity { NumberAnimation { duration: 150 } }
                                Text { text: "-"; anchors.centerIn: parent; color: replayDurationSpin.down.pressed ? Theme.borderDefault : Theme.textPrimary }
                            }
                            up.indicator: Item {
                                x: parent.width - width; y: 0; width: 20; height: parent.height
                                opacity: hoverHandler.hovered ? 1.0 : 0.0
                                Behavior on opacity { NumberAnimation { duration: 150 } }
                                Text { text: "+"; anchors.centerIn: parent; color: replayDurationSpin.up.pressed ? Theme.borderDefault : Theme.textPrimary }
                            }

                            contentItem: TextInput {
                                text: replayDurationSpin.textFromValue(replayDurationSpin.value, replayDurationSpin.locale)
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignVCenter
                                color: Theme.textPrimary
                                selectionColor: Theme.borderDefault
                                selectByMouse: true
                                readOnly: !replayDurationSpin.editable
                                validator: IntValidator { bottom: 1; top: 120 }
                            }

                            onValueModified: {
                                Backend.recording.setReplayDuration(value)
                                Backend.settings.replayDuration = value
                                Backend.settings.save()
                            }
                        }
                    }
                }

                // -- Waveform preview --
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.waveformHeight
                    color: Theme.recessedBackground; radius: 6; border.color: Theme.borderDefault
                    WaveformView {
                        id: replayWaveform
                        anchors.fill: parent; anchors.margins: 4
                        waveformData: Backend.replayWaveform
                        readOnly: true
                    }
                }

                // -- Save action --
                RowLayout {
                    Layout.fillWidth: true
                    Item { Layout.fillWidth: true }

                    ThemedButton {
                        id: saveReplayBtn
                        text: "Save Replay"
                        small: true
                        enabled: root.saveReplayEnabled
                        onClicked: {
                            if (!replayContent.isActive) return

                            var stamp = Utils.formatTimestamp(new Date())
                            var path = Backend.settings.saveDirectory + "/Replay_" + stamp + ".wav"

                            if (Backend.recording.saveReplay(path)) {
                                root.replaySaved(path)
                                root.setStatusText("Replay saved: Replay_" + stamp + ".wav")
                                root.playEnabled = true
                            } else {
                                root.setStatusText("Failed to save replay")
                            }
                        }
                    }
                }
            }
        }

        // ---- TRANSPORT CONTROLS ---------------------------------------
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: root.controlsRowHeight

                RowLayout {
                    anchors.centerIn: parent
                    spacing: root.controlButtonSpacing

                    Repeater {
                        model: [
                            { label: "● Rec", enabled: root.startEnabled, accent: Theme.accentRed, fontSize: 14, action: "start" },
                            { label: "■ Stop", enabled: root.stopEnabled, accent: Theme.borderDefault, fontSize: 14, action: "stop" },
                            { label: "▶ Play Recording", enabled: root.playEnabled, accent: Theme.borderDefault, fontSize: 11, action: "play" }
                        ]

                        delegate: Rectangle {
                            id: btnRoot
                            required property var modelData
                            readonly property bool isRecButton: modelData.action === "start"

                            width: root.controlButtonWidth
                            height: root.controlButtonHeight
                            radius: 4
                            opacity: modelData.enabled ? 1.0 : 0.5
                            border.color: isRecButton ? Theme.accentRed : Theme.borderDefault
                            color: {
                                if (!modelData.enabled) return "transparent"
                                if (mouseArea.pressed) return isRecButton ? Theme.accentRed : Theme.borderDefault
                                if (mouseArea.hovered) return isRecButton ? Qt.rgba(Theme.accentRed.r, Theme.accentRed.g, Theme.accentRed.b, 0.1) : Theme.recessedBackground
                                return Theme.appBackground
                            }

                            Text {
                                text: modelData.label
                                anchors.centerIn: parent
                                color: modelData.enabled ? (isRecButton ? Theme.accentRed : Theme.textPrimary) : Theme.textDim
                                font.bold: isRecButton
                                font.pixelSize: modelData.fontSize
                            }

                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: modelData.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onClicked: {
                                    if (!modelData.enabled) return
                                    switch (modelData.action) {
                                    case "start": root.startRequested(); break
                                    case "stop": root.stopRequested(); break
                                    case "play":
                                        if (root.lastRecordingPath.length > 0) Backend.playFile(root.lastRecordingPath)
                                        break
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                visible: root.lastRecordingPath.length > 0
                text: "Last: " + root.lastRecordingPath.split("/").pop()
                color: Theme.textSecondary
                font.pixelSize: 12
            }
        }
    }
}
