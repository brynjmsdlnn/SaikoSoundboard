import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Dialogs
import Saiko 1.0
import "shared/utils.js" as Utils

ApplicationWindow {
    id: app
    visible: true
    width: 1100
    height: 950
    minimumWidth: 1100
    minimumHeight: 600
    title: "Saiko Soundboard"
    color: Theme.appBackground
    flags: Qt.Window | Qt.FramelessWindowHint

    property string captureMode: "global"
    property string lastRecordingPath: ""
    property int lastPlaybackType: Backend.PlaybackNone
    property bool isWindowMoving: false

    property int normalWidth: 1100
    property int normalHeight: 900

    onWidthChanged: {
        if (visibility === Window.Windowed || visibility === Window.AutomaticVisibility) {
            normalWidth = width;
        }
    }
    onHeightChanged: {
        if (visibility === Window.Windowed || visibility === Window.AutomaticVisibility) {
            normalHeight = height;
        }
    }

    // Named capture states instead of bare 0/1/2/3 sprinkled through
    // onCaptureStateChanged. Mirrors whatever CaptureState enum the backend
    // actually uses - keep these in sync if that enum changes.
    readonly property int stateIdle: 0
    readonly property int stateReplay: 1
    readonly property int stateRecording: 2
    readonly property int stateRecordingAndReplay: 3

    function startRecording() {
        lastRecordingPath = Backend.generateRecordingFilePath();

        if (!Backend.recording.isEngineRunning)
            Backend.recording.startEngine(captureMode);

        if (!Backend.recording.startRecording(lastRecordingPath))
            return;
        recordingPanel.notifyRecordingStarted();
    }

    function stopRecording() {
        recordingPanel.notifyRecordingStopped();
        Backend.recording.stopRecording();

        var fileSize = Backend.recordingFileSize();
        if (fileSize > 100) {
            renameDialog.open();
        } else {
            recordingPanel.setStatusText("Recording failed or was empty");
            resetAfterStop();
        }
    }

    function finishRename(newName) {
        var dir = lastRecordingPath.substring(0, lastRecordingPath.lastIndexOf("/"));
        var currentFilename = lastRecordingPath.substring(lastRecordingPath.lastIndexOf("/") + 1);
        var dot = currentFilename.lastIndexOf(".");
        var currentName = dot > 0 ? currentFilename.substring(0, dot) : currentFilename;

        if (newName === currentName) {
            recordingPanel.setStatusText("Saved: " + currentFilename);
            recordingPanel.setPlayEnabled(true);
            app.lastPlaybackType = Backend.PlaybackRecording;
            resetAfterStop();
            return;
        }

        var finalPath = Backend.renameRecordingFile(lastRecordingPath, dir, newName);
        lastRecordingPath = finalPath;
        recordingPanel.setStatusText("Saved: " + lastRecordingPath.substring(lastRecordingPath.lastIndexOf("/") + 1));
        recordingPanel.setPlayEnabled(true);
        app.lastPlaybackType = Backend.PlaybackRecording;
        resetAfterStop();
    }

    function resetAfterStop() {
        recordingPanel.resetUI();
    }

    SaikoDialog {
        id: renameDialog
        title: "Save recording"
        text: "Enter a name for the recording:"
        confirmText: "Save"
        confirmColor: Theme.accentPurple
        width: 360

        Rectangle {
            Layout.fillWidth: true
            height: 36
            color: Theme.recessedBackground
            radius: Theme.borderRadius
            border.color: renameInput.activeFocus ? Theme.accentPurple : Theme.borderDefault
            border.width: 1
            Behavior on border.color {
                ColorAnimation {
                    duration: Theme.animDuration
                }
            }
            TextInput {
                id: renameInput
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                verticalAlignment: TextInput.AlignVCenter
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeNormal
                selectByMouse: true
                onAccepted: renameDialog.accept()
            }
        }

        onAccepted: {
            var name = renameInput.text.trim();
            finishRename(name.length > 0 ? name : "Recording");
        }

        onRejected: {
            Backend.deleteRecordingFile(lastRecordingPath);
            lastRecordingPath = "";
            resetAfterStop();
        }

        onOpened: {
            renameInput.forceActiveFocus();
        }

        onVisibleChanged: {
            if (visible) {
                var parts = lastRecordingPath.split("/");
                var filename = parts[parts.length - 1] || "";
                var dot = filename.lastIndexOf(".");
                renameInput.text = dot > 0 ? filename.substring(0, dot) : filename;
            }
        }
    }

    // ============================================================
    // Custom Title Bar
    // ============================================================
    TitleBar {
        id: titleBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        logViewerActive: app.showLogViewer
        showDeveloperOptions: app.devModeEnabled
        onSettingsClicked: settingsDialog.show()
        onAboutClicked: aboutDialog.show()
        onLogViewerClicked: app.toggleLogViewer()
    }

    // ============================================================
    // Main layout
    // ============================================================
    SplitView {
        id: verticalSplit
        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        orientation: Qt.Vertical

        handle: Rectangle {
            implicitHeight: 4
            color: SplitHandle.pressed ? Theme.accentPurple : (SplitHandle.hovered ? Theme.borderHover : Theme.borderDefault)
            Behavior on color {
                ColorAnimation {
                    duration: 120
                }
            }
        }

        SplitView {
            id: horizontalSplit
            SplitView.fillHeight: false
            SplitView.minimumHeight: 380
            orientation: Qt.Horizontal

            handle: Rectangle {
                implicitWidth: 4
                color: SplitHandle.pressed ? Theme.accentPurple : (SplitHandle.hovered ? Theme.borderHover : Theme.borderDefault)
                Behavior on color {
                    ColorAnimation {
                        duration: 120
                    }
                }
            }

            // ---------------- LEFT PANEL ----------------
            RecordingPanel {
                id: recordingPanel

                SplitView.preferredWidth: Backend.settings.leftPanelWidth > 0 ? Backend.settings.leftPanelWidth : 800
                SplitView.minimumWidth: sourcesDock.visible ? horizontalSplit.width - 500 - 4 : 300
                captureMode: app.captureMode
                lastRecordingPath: app.lastRecordingPath
                lastPlaybackType: app.lastPlaybackType

                onWidthChanged: {
                    if (width > 0) {
                        Backend.settings.leftPanelWidth = width;
                        Backend.settings.save();
                    }
                }

                onStartRequested: app.startRecording()
                onStopRequested: app.stopRecording()
                onCaptureModeSelected: function (newMode) {
                    app.captureMode = newMode;
                    sourcesDock.visible = (newMode === "multi");
                }
                onReplaySaved: function (path) {
                    app.lastRecordingPath = path;
                    app.lastPlaybackType = Backend.PlaybackReplay;
                    Backend.loadRecordingWaveform(path);
                }
                onSettingsRequested: settingsDialog.show()
                onAboutRequested: aboutDialog.show()
                onAssignToSlotRequested: assignToSlotDialog.show()
            }

            // ---------------- SOURCES DOCK ----------------
            Rectangle {
                id: sourcesDock
                SplitView.preferredWidth: Backend.settings.sourcesDockWidth > 0 ? Backend.settings.sourcesDockWidth : 300
                SplitView.minimumWidth: 300
                color: Theme.appBackground
                visible: (captureMode === "multi")

                onWidthChanged: {
                    if (width > 0 && visible) {
                        Backend.settings.sourcesDockWidth = Math.min(width, 500);
                        Backend.settings.save();
                    }
                }

                SourcesPanel {
                    id: sourcesPanel
                    anchors.fill: parent
                    anchors.margins: 12
                    sourceModel: Backend.sourceModel
                    locked: false

                    onSourceAdded: function (name, executableName, executablePath) {
                        Backend.sourceModel.addSource(name, executableName, executablePath);
                    }
                    onDeviceAdded: function (name, deviceName) {
                        Backend.sourceModel.addDeviceSource(name, deviceName);
                    }
                    onSourceRemoved: function (sourceId) {
                        Backend.sourceModel.removeSource(sourceId);
                    }
                }
            }
        }

        // ---------------- SOUNDBOARD SLOTS ----------------
        Rectangle {
            id: soundboardDock
            SplitView.preferredHeight: Backend.settings.soundboardDockHeight > 0 ? Backend.settings.soundboardDockHeight : 280
            SplitView.minimumHeight: 520
            color: Theme.recessedBackground
            border.color: Theme.borderDefault
            border.width: 1

            onHeightChanged: {
                if (height > 0) {
                    Backend.settings.soundboardDockHeight = height;
                    Backend.settings.save();
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

                SoundboardGridView {
                    id: soundboardGrid
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onRoutingSettingsRequested: {
                        settingsDialog.activeTab = "routing"
                        settingsDialog.show()
                    }
                    onOpenHotkeyRequested: function(slotId, playKey, assignKey) {
                        hotkeyDialog.openForSlot(slotId, playKey, assignKey);
                    }
                }
            }
        }
    }

    property bool showLogViewer: false
    property bool devModeEnabled: false
    property int folderPickerTarget: 0

    function toggleLogViewer() {
        if (logWindow.visible) {
            logWindow.close();
        } else {
            logWindow.show();
        }
    }

    FolderDialog {
        id: folderDialog
        title: "Please choose a folder"
        onAccepted: {
            let folderUrl = selectedFolder.toString();
            let path = folderUrl.replace(/^(file:\/{2,3})/, "");
            switch (folderPickerTarget) {
            case 0:
                Backend.settings.recordingDirectoryOverride = path;
                break;
            case 1:
                Backend.settings.replayDirectoryOverride = path;
                break;
            case 2:
                Backend.settings.baseDirectory = path;
                break;
            }
            Backend.settings.save();
        }
    }

    function openFolderPicker(initialPath) {
        let url = initialPath;
        if (url && !url.startsWith("file://")) {
            url = "file:///" + url;
        }
        folderDialog.currentFolder = url;
        folderDialog.open();
    }

    SettingsDialog {
        id: settingsDialog
        onChangeBaseRequested: {
            folderPickerTarget = 2;
            openFolderPicker(Backend.settings.baseDirectory);
        }
        onChangeRecordingRequested: {
            folderPickerTarget = 0;
            openFolderPicker(Backend.settings.recordingDirectory);
        }
        onChangeReplayRequested: {
            folderPickerTarget = 1;
            openFolderPicker(Backend.settings.replayDirectory);
        }
        onResetRecordingRequested: {
            Backend.settings.recordingDirectoryOverride = "";
            Backend.settings.save();
        }
        onResetReplayRequested: {
            Backend.settings.replayDirectoryOverride = "";
            Backend.settings.save();
        }
    }

    AboutDialog {
        id: aboutDialog
        onDevModeTriggered: {
            app.devModeEnabled = true;
            aboutDialog.close();
        }
    }

    LogWindow {
        id: logWindow
        onOpened: showLogViewer = true
        onClosed: showLogViewer = false
    }

    AssignToSlotDialog {
        id: assignToSlotDialog
        filePath: app.lastRecordingPath
        onAccepted: {
            recordingPanel.setStatusText("Assigned to: " + selectedSlotName);
        }
    }

    HotkeyDialog {
        id: hotkeyDialog
        onAccepted: {
            Backend.soundboard.setHotkeys(hotkeyDialog.slotId, hotkeyDialog.playKey, hotkeyDialog.assignKey);
        }
    }

    Connections {
        target: Backend
        function onCaptureStateChanged(state) {
            var isReplay = (state === app.stateReplay || state === app.stateRecordingAndReplay);

            recordingPanel.setModeEnabled(state === app.stateIdle);
            sourcesPanel.locked = (state !== app.stateIdle);

            recordingPanel.setSaveReplayEnabled(isReplay);
            recordingPanel.setReplayChecked(isReplay);

            switch (state) {
            case app.stateIdle:
                recordingPanel.setStatusText("Ready");
                break;
            case app.stateReplay:
                recordingPanel.setStatusText("Background replay active...");
                break;
            case app.stateRecording:
                recordingPanel.setStatusText("Manual recording active...");
                break;
            case app.stateRecordingAndReplay:
                recordingPanel.setStatusText("Recording + replay active...");
                break;
            }
        }
        function onPlaybackStateChanged() {
            if (!Backend.isPlaying) {
                recordingPanel.setStatusText("Ready");
                recordingPanel.setStartEnabled(true);
                recordingPanel.setPlayEnabled(app.lastRecordingPath.length > 0);
            }
        }
    }

    // ============================================================
    // Window Resize Handlers (only enabled when not maximized)
    // ============================================================
    WindowResizeHandlers {
        id: resizeHandlers
        anchors.fill: parent
    }
}
