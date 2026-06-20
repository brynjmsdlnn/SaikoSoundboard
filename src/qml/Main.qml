import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Dialogs
import Saiko 1.0

ApplicationWindow {
    id: app
    visible: true
    width: 1100
    height: 700
    minimumWidth: 900
    minimumHeight: 600
    title: "Saiko Soundboard"
    color: Theme.appBackground

    property string captureMode: "global"
    property bool isRecording: false
    property bool isReplayActive: false
    property int remainingSec: 0
    property real elapsedSec: 0.0
    property string lastRecordingPath: ""

    function startRecording() {
        var ts = new Date()
        var fmt = ts.getFullYear() +
            ("0" + (ts.getMonth()+1)).slice(-2) +
            ("0" + ts.getDate()).slice(-2) + "_" +
            ("0" + ts.getHours()).slice(-2) +
            ("0" + ts.getMinutes()).slice(-2) +
            ("0" + ts.getSeconds()).slice(-2)
        lastRecordingPath = Backend.settings.saveDirectory + "/Recording_" + fmt + ".wav"

        if (!Backend.recording.isEngineRunning)
            Backend.recording.startEngine(captureMode)

        if (!Backend.recording.startRecording(lastRecordingPath))
            return

        remainingSec = 10
        elapsedSec = 0.0
        isRecording = true
        playBtn.enabled = false
        stopBtn.enabled = true
        startBtn.enabled = false
        timerLabel.text = "Time remaining: 10s"
        statsLabel.text = "Size: 0 KB \u00b7 Time: 0.0s"
        recordingTimer.start()
        stopTimer.start()
    }

    function stopRecording() {
        recordingTimer.stop()
        stopTimer.stop()
        Backend.recording.stopRecording()

        var fileSize = Backend.recordingFileSize()
        if (fileSize > 100) {
            renameDialog.open()
        } else {
            statusLabel.text = "Recording failed or was empty"
            resetAfterStop()
        }
    }

    function finishRename(newName) {
        var dir = lastRecordingPath.substring(0, lastRecordingPath.lastIndexOf("/"))
        var finalPath = Backend.renameRecordingFile(lastRecordingPath, dir, newName)
        lastRecordingPath = finalPath
        statusLabel.text = "Saved: " + lastRecordingPath.substring(lastRecordingPath.lastIndexOf("/") + 1)
        playBtn.enabled = true
        resetAfterStop()
    }

    function resetAfterStop() {
        isRecording = false
        timerLabel.text = ""
        stopBtn.enabled = false
        startBtn.enabled = true
    }

    Timer {
        id: recordingTimer
        interval: 100
        repeat: true
        onTriggered: {
            elapsedSec += 0.1
            remainingSec = Math.max(0, 10 - Math.floor(elapsedSec))
            timerLabel.text = "Time remaining: " + remainingSec + "s"
            var bytes = Backend.recordingFileSize()
            statsLabel.text = "Size: " + Math.round(bytes/1024) + " KB \u00b7 Time: " + elapsedSec.toFixed(1) + "s"
        }
    }

    Timer {
        id: stopTimer
        interval: 10000
        repeat: false
        onTriggered: stopRecording()
    }

    // ============================================================
    // Reusable section card wrapper
    // ============================================================
    component SectionCard: Rectangle {
            id: section
            default property alias content: sectionLayout.children
            property string heading: ""

            Layout.fillWidth: true
            Layout.preferredHeight: sectionLayout.implicitHeight + 28
            color: Theme.cardBackground
            radius: Theme.cardRadius
            border.color: Theme.borderDefault
            border.width: 1

            ColumnLayout {
                id: sectionLayout
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10

                Text {
                    visible: section.heading !== ""
                    text: section.heading
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSizeSmall
                    font.letterSpacing: 1.2
                    font.weight: Font.Bold
                }
            }
        }



    Dialog {
        id: renameDialog
        title: "Save recording"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        x: Math.round((app.width - width) / 2)
        y: Math.round((app.height - height) / 2)
        width: 360

        background: Rectangle {
            color: Theme.inputBackground
            border.color: Theme.borderHover
            border.width: 1
            radius: Theme.cardRadius
        }

        contentItem: ColumnLayout {
            anchors.margins: 16
            spacing: 12
            Text {
                text: "Enter a name for the recording:"
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeHeading
            }
            Rectangle {
                Layout.fillWidth: true
                height: 36
                color: Theme.appBackground
                radius: Theme.borderRadius
                border.color: renameInput.activeFocus ? Theme.accentPurple : Theme.borderDefault
                border.width: 1
                Behavior on border.color { ColorAnimation { duration: Theme.animDuration } }
                TextInput {
                    id: renameInput
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    verticalAlignment: TextInput.AlignVCenter
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeNormal
                    focus: true
                    onAccepted: renameDialog.accept()
                }
            }
        }

        onAccepted: {
            var name = renameInput.text.trim()
            finishRename(name.length > 0 ? name : "Recording")
        }
        onRejected: {
            resetAfterStop()
        }
        onVisibleChanged: {
            if (visible) renameInput.text = ""
        }
    }

    // ============================================================
    // Main layout
    // ============================================================
    SplitView {
        id: verticalSplit
        anchors.fill: parent
        orientation: Qt.Vertical

        handle: Rectangle {
            implicitHeight: 4
            color: SplitHandle.pressed ? Theme.accentPurple : (SplitHandle.hovered ? Theme.borderHover : Theme.borderDefault)
            Behavior on color { ColorAnimation { duration: 120 } }
        }

        SplitView {
            id: horizontalSplit
            SplitView.fillHeight: true
            orientation: Qt.Horizontal

            handle: Rectangle {
                implicitWidth: 4
                color: SplitHandle.pressed ? Theme.accentPurple : (SplitHandle.hovered ? Theme.borderHover : Theme.borderDefault)
                Behavior on color { ColorAnimation { duration: 120 } }
            }

            // ---------------- LEFT PANEL ----------------
            Flickable {
                id: leftPanelFlickable
                SplitView.preferredWidth: Backend.settings.leftPanelWidth > 0 ? Backend.settings.leftPanelWidth : 340
                SplitView.minimumWidth: 300
                contentWidth: width
                contentHeight: leftColumn.implicitHeight + 32
                clip: true

                onWidthChanged: {
                    if (width > 0) {
                        Backend.settings.leftPanelWidth = width
                        Backend.settings.save()
                    }
                }

                ColumnLayout {
                    id: leftColumn
                    x: Math.max(16, (parent.width - width) / 2)
                    y: 16
                    width: Math.min(parent.width - 32, 420)
                    spacing: 14

                    Text {
                        text: "Saiko Soundboard"
                        color: Theme.textPrimary
                        font.pixelSize: 18
                        font.weight: Font.Bold
                    }

                    // --- Status section ---
                    SectionCard {
                        heading: "STATUS"

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                    Text {
                        id: statusLabel
                        text: "Ready"
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeHeading
                    }
                    Text {
                        id: timerLabel
                        text: ""
                        color: Theme.accentPurple
                        font.pixelSize: Theme.fontSizeHeading
                        font.weight: Font.Medium
                        visible: text !== ""
                    }
                    Text {
                        id: statsLabel
                        text: "Size: 0 KB \u00b7 Time: 0s"
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSizeSmall
                    }
                        }
                    }

                    // --- Capture settings section ---
                    SectionCard {
                        heading: "CAPTURE"

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                    Text {
                        text: "Mode"
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeNormal
                        Layout.preferredWidth: 60
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        color: Theme.appBackground
                        radius: Theme.borderRadius
                        border.color: Theme.borderDefault
                        border.width: 1

                        ComboBox {
                            id: modeCombo
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            model: [
                                { text: "System output (global)", value: "global" },
                                { text: "Multi-track (sources)", value: "multi" }
                            ]
                            textRole: "text"
                            valueRole: "value"
                            currentIndex: 0
                            background: Item {}
                            contentItem: Text {
                                text: {
                                    for (var i = 0; i < modeCombo.model.length; i++) {
                                        if (modeCombo.model[i].value === modeCombo.currentValue)
                                            return modeCombo.model[i].text
                                    }
                                    return ""
                                }
                                color: Theme.textPrimary
                                font.pixelSize: Theme.fontSizeNormal
                                verticalAlignment: Text.AlignVCenter
                            }
                                    onActivated: {
                                        captureMode = modeCombo.currentValue
                                        sourcesDock.visible = (captureMode === "multi")
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 6

                    Text {
                        text: "Save to"
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeNormal
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Rectangle {
                            Layout.fillWidth: true
                            height: 32
                            color: Theme.appBackground
                            radius: Theme.borderRadius
                            border.color: Theme.borderDefault
                            border.width: 1
                            TextInput {
                                id: saveDirInput
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                verticalAlignment: TextInput.AlignVCenter
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSizeSmall
                                readOnly: true
                                text: Backend.settings.saveDirectory
                            }
                        }
                        ThemedButton { text: "Open"; small: true; onClicked: Qt.openUrlExternally("file:///" + Backend.settings.saveDirectory) }
                        ThemedButton { text: "Change..."; small: true; onClicked: folderDialog.open() }
                    }
                        }
                    }

                    // --- Replay buffer section ---
                    SectionCard {
                        heading: "REPLAY BUFFER"

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            CustomCheckBox {
                                id: replayCheck
                                text: "Enabled"
                                checked: Backend.settings.replayEnabled
                                onToggled: {
                                    Backend.settings.replayEnabled = checked
                                    Backend.settings.save()
                                    Backend.recording.setReplayEnabled(checked, captureMode)
                                }
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: "Duration"
                                color: Theme.textSecondary
                                font.pixelSize: Theme.fontSizeSmall
                            }
                            Rectangle {
                                width: 60
                                height: 28
                                color: Theme.appBackground
                                radius: Theme.borderRadius
                                border.color: Theme.borderDefault
                                border.width: 1
                                SpinBox {
                                    id: replayDurationSpin
                                    anchors.fill: parent
                                    anchors.leftMargin: 4
                                    anchors.rightMargin: 4
                                    from: 1; to: 120
                                    value: Backend.settings.replayDuration
                                    background: Item {}
                                    contentItem: Text {
                                        text: replayDurationSpin.value + "s"
                                        color: Theme.textPrimary
                                        font.pixelSize: Theme.fontSizeSmall
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                    onValueModified: {
                                        Backend.recording.setReplayDuration(value)
                                        Backend.settings.replayDuration = value
                                        Backend.settings.save()
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 60
                            color: Theme.recessedBackground
                            radius: Theme.cardRadius
                            border.color: Theme.borderDefault
                            border.width: 1
                            clip: true

                            WaveformView {
                                id: replayWaveform
                                anchors.fill: parent
                                anchors.margins: 3
                                waveformData: Backend.replayWaveform
                                readOnly: true
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                    Text {
                            id: replayStatusText
                            text: isReplayActive ? "Status: active" : "Status: inactive"
                            color: Theme.textDim
                            font.pixelSize: Theme.fontSizeSmall
                        }
                        Item { Layout.fillWidth: true }
                        ThemedButton {
                            id: saveReplayBtn
                            text: "Save replay"
                            small: true
                            enabled: isReplayActive
                                onClicked: {
                                    if (isReplayActive) {
                                        var ts = new Date()
                                        var fmt = ts.getFullYear() +
                                            ("0" + (ts.getMonth()+1)).slice(-2) +
                                            ("0" + ts.getDate()).slice(-2) + "_" +
                                            ("0" + ts.getHours()).slice(-2) +
                                            ("0" + ts.getMinutes()).slice(-2) +
                                            ("0" + ts.getSeconds()).slice(-2)
                                        var path = Backend.settings.saveDirectory + "/Replay_" + fmt + ".wav"
                                        if (Backend.recording.saveReplay(path)) {
                                            statusLabel.text = "Replay saved: Replay_" + fmt + ".wav"
                                            lastRecordingPath = path
                                            playBtn.enabled = true
                                        } else {
                                            statusLabel.text = "Failed to save replay or buffer empty"
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // --- Transport controls ---
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        ThemedButton {
                            id: startBtn
                            text: "Start recording"
                            accentColor: Theme.accentGreen
                            filled: true
                            onClicked: startRecording()
                        }
                        ThemedButton {
                            id: stopBtn
                            text: "Stop"
                            accentColor: Theme.accentRed
                            filled: true
                            enabled: false
                            onClicked: stopRecording()
                        }
                        ThemedButton {
                            id: playBtn
                            text: "Play last"
                            accentColor: Theme.accentPurple
                            filled: true
                            enabled: false
                            onClicked: {
                                if (lastRecordingPath.length > 0) Backend.playFile(lastRecordingPath)
                            }
                        }
                    }

                    Item { Layout.preferredHeight: 8 }
                }
            }

            // ---------------- SOURCES DOCK ----------------
            Rectangle {
                id: sourcesDock
                SplitView.preferredWidth: Backend.settings.sourcesDockWidth > 0 ? Backend.settings.sourcesDockWidth : 320
                SplitView.minimumWidth: 260
                color: Theme.appBackground
                visible: (captureMode === "multi")

                onWidthChanged: {
                    if (width > 0 && visible) {
                        Backend.settings.sourcesDockWidth = width
                        Backend.settings.save()
                    }
                }

                SourcesPanel {
                    id: sourcesPanel
                    anchors.fill: parent
                    anchors.margins: 12
                    sourceModel: Backend.sourceModel
                    locked: !startBtn.enabled

                    onSourceAdded: function(name, executableName, executablePath) {
                        Backend.sourceModel.addSource(name, executableName, executablePath)
                    }
                    onSourceRemoved: function(sourceId) {
                        Backend.sourceModel.removeSource(sourceId)
                    }
                }
            }
        }

        // ---------------- BOTTOM DOCK: SOUNDBOARD SLOTS ----------------
        Rectangle {
            id: soundboardDock
            SplitView.preferredHeight: Backend.settings.soundboardDockHeight > 0 ? Backend.settings.soundboardDockHeight : 280
            SplitView.minimumHeight: 180
            color: Theme.recessedBackground
            border.color: Theme.borderDefault
            border.width: 1

            onHeightChanged: {
                if (height > 0) {
                    Backend.settings.soundboardDockHeight = height
                    Backend.settings.save()
                }
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: Theme.borderDefault
                }

                SoundboardPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }

    FileDialog {
        id: folderDialog
        title: "Select save directory"
        fileMode: FileDialog.Directory
        currentFolder: "file:///" + Backend.settings.saveDirectory
        onAccepted: {
            var path = selectedFile.toString()
            if (path.startsWith("file:///"))
                path = path.substring(8)
            Backend.settings.saveDirectory = path
            Backend.settings.save()
            saveDirInput.text = path
        }
    }

    Connections {
        target: Backend
        function onCaptureStateChanged(state) {
            var isRecordingState = (state === 2 || state === 3)
            var isReplay = (state === 1 || state === 3)
            isReplayActive = isReplay

            modeCombo.enabled = (state === 0)
            sourcesPanel.locked = !modeCombo.enabled

            replayStatusText.text = isReplayActive ? "Status: active" : "Status: inactive"
            saveReplayBtn.enabled = isReplayActive
            replayCheck.checked = isReplayActive

            if (state === 0) statusLabel.text = "Ready"
            else if (state === 1) statusLabel.text = "Background replay active..."
            else if (state === 2) statusLabel.text = "Manual recording active..."
            else if (state === 3) statusLabel.text = "Recording + replay active..."
        }
        function onPlaybackStateChanged() {
            if (!Backend.isPlaying) {
                statusLabel.text = "Ready"
                startBtn.enabled = true
                playBtn.enabled = (lastRecordingPath.length > 0)
            }
        }
    }
}
