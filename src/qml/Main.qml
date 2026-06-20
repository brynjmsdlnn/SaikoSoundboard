import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Dialogs
import Saiko 1.0
import "utils.js" as Utils

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
    property bool isReplayActive: false
    property string lastRecordingPath: ""

    function startRecording() {
        var fmt = Utils.formatTimestamp(new Date())
        lastRecordingPath = Backend.settings.saveDirectory + "/Recording_" + fmt + ".wav"

        if (!Backend.recording.isEngineRunning)
            Backend.recording.startEngine(captureMode)

        if (!Backend.recording.startRecording(lastRecordingPath))
            return

        recordingPanel.notifyRecordingStarted()
    }

    function stopRecording() {
        recordingPanel.notifyRecordingStopped()
        Backend.recording.stopRecording()

        var fileSize = Backend.recordingFileSize()
        if (fileSize > 100) {
            renameDialog.open()
        } else {
            recordingPanel.setStatusText("Recording failed or was empty")
            resetAfterStop()
        }
    }

    function finishRename(newName) {
        var dir = lastRecordingPath.substring(0, lastRecordingPath.lastIndexOf("/"))
        var currentFilename = lastRecordingPath.substring(lastRecordingPath.lastIndexOf("/") + 1)
        var dot = currentFilename.lastIndexOf(".")
        var currentName = dot > 0 ? currentFilename.substring(0, dot) : currentFilename

        if (newName === currentName) {
            recordingPanel.setStatusText("Saved: " + currentFilename)
            recordingPanel.setPlayEnabled(true)
            resetAfterStop()
            return
        }

        var finalPath = Backend.renameRecordingFile(lastRecordingPath, dir, newName)
        lastRecordingPath = finalPath
        recordingPanel.setStatusText("Saved: " + lastRecordingPath.substring(lastRecordingPath.lastIndexOf("/") + 1))
        recordingPanel.setPlayEnabled(true)
        resetAfterStop()
    }

    function resetAfterStop() {
        recordingPanel.resetUI()
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
            if (visible) {
                var parts = lastRecordingPath.split("/")
                var filename = parts[parts.length - 1] || ""
                var dot = filename.lastIndexOf(".")
                renameInput.text = dot > 0 ? filename.substring(0, dot) : filename
            }
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
            RecordingPanel {
                id: recordingPanel
                SplitView.preferredWidth: Backend.settings.leftPanelWidth > 0 ? Backend.settings.leftPanelWidth : 340
                captureMode: app.captureMode
                lastRecordingPath: app.lastRecordingPath

                onWidthChanged: {
                    if (width > 0) {
                        Backend.settings.leftPanelWidth = width
                        Backend.settings.save()
                    }
                }

                onStartRequested: app.startRecording()
                onStopRequested: app.stopRecording()
                onChangeFolderRequested: folderDialog.open()
                onCaptureModeSelected: function(newMode) {
                    sourcesDock.visible = (newMode === "multi")
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
                    locked: false

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
        }
    }

    Connections {
        target: Backend
        function onCaptureStateChanged(state) {
            var isRecordingState = (state === 2 || state === 3)
            var isReplay = (state === 1 || state === 3)
            isReplayActive = isReplay

            recordingPanel.setModeEnabled(state === 0)
            sourcesPanel.locked = !(state === 0)

            recordingPanel.setReplayStatusText(isReplayActive ? "Status: active" : "Status: inactive")
            recordingPanel.setSaveReplayEnabled(isReplayActive)
            recordingPanel.setReplayChecked(isReplayActive)

            if (state === 0) recordingPanel.setStatusText("Ready")
            else if (state === 1) recordingPanel.setStatusText("Background replay active...")
            else if (state === 2) recordingPanel.setStatusText("Manual recording active...")
            else if (state === 3) recordingPanel.setStatusText("Recording + replay active...")
        }
        function onPlaybackStateChanged() {
            if (!Backend.isPlaying) {
                recordingPanel.setStatusText("Ready")
                recordingPanel.setStartEnabled(true)
                recordingPanel.setPlayEnabled(lastRecordingPath.length > 0)
            }
        }
    }
}
