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
            onCaptureModeSelected: (newMode) => root.captureModeSelected(newMode)
            onSettingsRequested: root.settingsRequested()
            onAboutRequested: root.aboutRequested()
        }

        RecordingSection {
            id: recordingSection
            Layout.fillWidth: true
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
        }

        ReplayBufferSection {
            Layout.fillWidth: true
            captureMode: root.captureMode
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
}
