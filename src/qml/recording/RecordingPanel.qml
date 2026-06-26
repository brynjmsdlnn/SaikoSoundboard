import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0
import "../shared/utils.js" as Utils

Flickable {
    id: root

    property string captureMode: "global"
    property string lastRecordingPath: ""
    property bool startEnabled: true
    property bool stopEnabled: false
    property bool playEnabled: false
    property bool modeEnabled: true
    property bool saveReplayEnabled: false
    property bool replayChecked: false
    property string statusText: "Ready"

    signal startRequested
    signal stopRequested
    signal settingsRequested
    signal captureModeSelected(string newMode)
    signal replaySaved(string path)

    function notifyRecordingStarted() {
        startEnabled = false;
        stopEnabled = true;
        playEnabled = false;
        recordingHeader.notifyRecordingStarted();
    }
    function notifyRecordingStopped() {
        recordingHeader.notifyRecordingStopped();
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
        recordingHeader.resetUI();
        stopEnabled = false;
        startEnabled = true;
    }

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

    SplitView.minimumWidth: 800
    SplitView.minimumHeight: 400
    contentWidth: width
    contentHeight: mainColumn.implicitHeight + 24
    clip: true

    ColumnLayout {
        id: mainColumn
        width: parent.width - 16
        x: root.contentMargin
        y: root.contentMargin
        spacing: root.sectionSpacing

        RecordingHeader {
            id: recordingHeader
            Layout.fillWidth: true
            lastRecordingPath: root.lastRecordingPath
            statusText: root.statusText
            startEnabled: root.startEnabled
            recordingDurationSec: root.recordingDurationSec
            cardPadding: root.cardPadding
            headerHeight: root.headerHeight
            onStopRequested: root.stopRequested()
        }

        CaptureModeBar {
            Layout.fillWidth: true
            modeEnabled: root.modeEnabled
            captureMode: root.captureMode
            cardPadding: root.cardPadding
            onCaptureModeSelected: (newMode) => root.captureModeSelected(newMode)
            onSettingsRequested: root.settingsRequested()
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

        TransportControls {
            Layout.fillWidth: true
            startEnabled: root.startEnabled
            stopEnabled: root.stopEnabled
            playEnabled: root.playEnabled
            lastRecordingPath: root.lastRecordingPath
            controlsRowHeight: root.controlsRowHeight
            onStartRequested: root.startRequested()
            onStopRequested: root.stopRequested()
        }
    }
}
