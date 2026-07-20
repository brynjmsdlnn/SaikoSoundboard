import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Item {
    id: root

    property string captureMode: "global"
    property string lastRecordingPath: ""
    property int lastPlaybackType: Backend.PlaybackNone
    property bool startEnabled: true
    property bool stopEnabled: false
    property bool playEnabled: false
    property bool modeEnabled: true
    property bool saveReplayEnabled: false
    property bool replayChecked: false
    property string statusText: "Ready"

    signal startRequested()
    signal stopRequested()
    signal settingsRequested()
    signal aboutRequested()
    signal captureModeSelected(string newMode)
    signal replaySaved(string path)
    signal assignToSlotRequested()
    signal openSourceSelectionRequested()

    function notifyRecordingStarted() {
        startEnabled = false;
        stopEnabled = true;
        playEnabled = false;
    }
    function notifyRecordingStopped() {
        stopEnabled = false;
        startEnabled = true;
    }
    function setStatusText(t) {
        statusText = t;
    }
    function setPlayEnabled(e) {
        playEnabled = e;
    }
    function setStartEnabled(e) {
        startEnabled = e;
    }
    function setModeEnabled(e) {
        modeEnabled = e;
    }
    function setReplayChecked(c) {
        replayChecked = c;
    }
    function setSaveReplayEnabled(e) {
        saveReplayEnabled = e;
    }
    function resetUI() {
        stopEnabled = false;
        startEnabled = true;
    }

    readonly property int recordingDurationSec: 10
    readonly property int contentMargin: 8
    readonly property int sectionSpacing: 10
    readonly property int cardPadding: 12
    readonly property int waveformHeight: 80
    readonly property int pulseDurationMs: 800

    // Token to force re-evaluation of the isCaptureReady binding when sources change.
    property int __captureToken: 0

    /// Capture readiness: true for global mode, checks source model for multi mode.
    readonly property bool isCaptureReady: {
        __captureToken; // capture dependency for re-evaluation
        return Backend.recording.isCaptureModeReady(root.captureMode);
    }

    Connections {
        target: Backend.recording
        function onCaptureReadyChanged() { root.__captureToken++; }
    }

    SplitView.minimumWidth: 800
    SplitView.minimumHeight: 400

    ColumnLayout {
        id: mainColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: root.contentMargin
        spacing: root.sectionSpacing

        RecordingHeader {
            id: recordingHeader
            Layout.fillWidth: true
            startEnabled: root.startEnabled
            cardPadding: root.cardPadding
            captureMode: root.captureMode
            modeEnabled: root.modeEnabled
            statusText: root.statusText
            onCaptureModeSelected: (newMode) => root.captureModeSelected(newMode)
            onSettingsRequested: root.settingsRequested()
            onAboutRequested: root.aboutRequested()
        }

        // Capture sections container wrapper
        Item {
            id: sectionsWrapper
            Layout.fillWidth: true
            implicitHeight: sectionsColumn.implicitHeight

            ColumnLayout {
                id: sectionsColumn
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: root.sectionSpacing

                opacity: root.isCaptureReady ? 1.0 : 0.4
                Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }

                RecordingSection {
                    id: recordingSection
                    Layout.fillWidth: true
                    captureMode: root.captureMode
                    isCaptureReady: root.isCaptureReady
                    lastRecordingPath: root.lastRecordingPath
                    lastPlaybackType: root.lastPlaybackType
                    startEnabled: root.startEnabled
                    stopEnabled: root.stopEnabled
                    playEnabled: root.playEnabled
                    recordingDurationSec: root.recordingDurationSec
                    waveformHeight: root.waveformHeight
                    cardPadding: root.cardPadding
                    pulseDurationMs: root.pulseDurationMs
                    onStartRequested: root.startRequested()
                    onStopRequested: root.stopRequested()
                    onAssignToSlotRequested: root.assignToSlotRequested()
                }

                ReplayBufferSection {
                    Layout.fillWidth: true
                    captureMode: root.captureMode
                    isCaptureReady: root.isCaptureReady
                    replayChecked: root.replayChecked
                    saveReplayEnabled: root.saveReplayEnabled
                    pulseDurationMs: root.pulseDurationMs
                    waveformHeight: root.waveformHeight
                    cardPadding: root.cardPadding
                    onReplaySaved: function(path) {
                        root.replaySaved(path);
                        root.playEnabled = true;
                    }
                    onStatusMessage: (text) => root.setStatusText(text)
                }
            }

            // Multi-track setup overlay (blocks interaction + explains missing sources)
            CaptureSetupOverlay {
                anchors.fill: parent
                active: !root.isCaptureReady
                title: "No Audio Sources"
                description: "Multi-track capture requires at least one audio source before recording or replay buffer can begin."
                buttonText: "Add Audio Source"
                onActionRequested: root.openSourceSelectionRequested()
            }
        }
    }
}
