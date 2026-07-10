import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Rectangle {
    id: root

    property string lastRecordingPath: ""
    property int lastPlaybackType: Backend.PlaybackNone
    property bool startEnabled: true
    property bool stopEnabled: false
    property bool playEnabled: false
    property int recordingDurationSec: 10
    property int waveformHeight: 80
    property int cardPadding: 12
    property int pulseDurationMs: 800

    signal startRequested()
    signal stopRequested()

    implicitHeight: sectionContent.implicitHeight + 24
    
    // Dynamic background and border color matching the current state
    color: root.stopEnabled ? Qt.rgba(Theme.accentRed.r, Theme.accentRed.g, Theme.accentRed.b, 0.04)
                            : (Backend.isPlaying ? (root.lastPlaybackType === Backend.PlaybackReplay ? Qt.rgba(Theme.accentGreen.r, Theme.accentGreen.g, Theme.accentGreen.b, 0.04) : Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.04)) : Theme.appBackground)
    
    border.color: root.stopEnabled ? Theme.accentRed
                                   : (Backend.isPlaying ? (root.lastPlaybackType === Backend.PlaybackReplay ? Theme.accentGreen : Theme.accentPurple) : Theme.borderDefault)
    
    radius: Theme.cardRadius

    Behavior on color {
        ColorAnimation { duration: 300 }
    }
    Behavior on border.color {
        ColorAnimation { duration: 300 }
    }

    property real __elapsedSec: 0.0
    property int __recordedBytes: 0

    Timer {
        id: recordingStatsTimer
        interval: 100
        repeat: true
        running: root.stopEnabled
        onTriggered: {
            root.__elapsedSec += 0.1;
            root.__recordedBytes = Backend.recordingFileSize();
        }
        onRunningChanged: {
            if (running) {
                root.__elapsedSec = 0.0;
                root.__recordedBytes = 0;
            }
        }
    }

    Timer {
        id: autoStopTimer
        interval: root.recordingDurationSec * 1000
        repeat: false
        running: root.stopEnabled
        onTriggered: root.stopRequested()
    }

    ColumnLayout {
        id: sectionContent
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: root.cardPadding
        spacing: 8

        // Row 1: Header (Full Width)
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Rectangle {
                id: statusDot
                width: 8
                height: 8
                radius: 4
                color: root.stopEnabled ? Theme.accentRed : (Backend.isPlaying ? (root.lastPlaybackType === Backend.PlaybackReplay ? Theme.accentGreen : Theme.accentPurple) : Theme.textDim)
                opacity: 1.0

                SequentialAnimation on opacity {
                    running: root.stopEnabled || Backend.isPlaying
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.3; duration: root.pulseDurationMs; easing.type: Easing.InOutQuad }
                    NumberAnimation { to: 1.0; duration: root.pulseDurationMs; easing.type: Easing.InOutQuad }
                }

                Connections {
                    target: root
                    function onStopEnabledChanged() {
                        if (!root.stopEnabled && !Backend.isPlaying)
                            statusDot.opacity = 1.0;
                    }
                }
                Connections {
                    target: Backend
                    function onIsPlayingChanged() {
                        if (!root.stopEnabled && !Backend.isPlaying)
                            statusDot.opacity = 1.0;
                    }
                }
            }

            Text {
                text: "Recording"
                color: Theme.textPrimary
                font.bold: true
            }

            SaikoIconButton {
                iconSource: "image://icons/folder?color=%23b0b0b0"
                tooltipText: "Open Recordings Folder"
                onClicked: Qt.openUrlExternally("file:///" + encodeURI(Backend.settings.recordingDirectory))
            }

            Item {
                Layout.fillWidth: true
            }

            Text {
                id: infoLabel
                color: Theme.textSecondary
                font.pixelSize: 12
                Layout.alignment: Qt.AlignVCenter
                text: {
                    if (root.stopEnabled) {
                        return "Recording: " + root.__elapsedSec.toFixed(1) + "s (" + Math.round(root.__recordedBytes / 1024) + " KB)";
                    } else if (Backend.isPlaying) {
                        var pos = Backend.playbackPosition;
                        var dur = Backend.playbackDuration;
                        return "Playing: " + (pos / 1000).toFixed(1) + "s / " + (dur / 1000).toFixed(1) + "s";
                    } else if (root.lastRecordingPath !== "") {
                        var filename = root.lastRecordingPath.split("/").pop();
                        var labelPrefix = root.lastPlaybackType === Backend.PlaybackReplay ? "Last Replay: " : "Last Rec: ";
                        return labelPrefix + filename;
                    } else {
                        return "Ready";
                    }
                }
            }
        }

        // Row 2: Controls and Waveform inline side-by-side
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            // Left: Big Record/Stop Button (styled like Soundboard Slot Editor action buttons)
            Button {
                id: recButton
                implicitWidth: 80
                Layout.preferredHeight: 80
                enabled: root.startEnabled || root.stopEnabled
                opacity: enabled ? 1.0 : 0.4

                Behavior on opacity {
                    NumberAnimation { duration: 150 }
                }
                
                background: Rectangle {
                    color: parent.enabled && parent.hovered ? Theme.inputBackground : Theme.recessedBackground
                    radius: 8
                    border.color: parent.enabled && parent.hovered ? (root.stopEnabled ? Theme.accentRed : Theme.accentPurple) : Theme.borderDefault
                    border.width: 1
                }

                contentItem: ColumnLayout {
                    spacing: 6
                    Layout.alignment: Qt.AlignVCenter

                    Image {
                        source: "image://icons/" + (root.stopEnabled ? "square" : "circle") + "?color=%23" + (root.stopEnabled ? "e35d5d" : "b0b0b0")
                        sourceSize: Qt.size(24, 24)
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: root.stopEnabled ? "Stop" : "Record"
                        color: Theme.textPrimary
                        font.pixelSize: 12
                        font.weight: Font.Medium
                        Layout.alignment: Qt.AlignHCenter
                    }
                }

                // Pulsing background color animation on the button itself when recording is active
                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: Theme.accentRed
                    visible: root.stopEnabled
                    opacity: 0.15
                    z: -1

                    SequentialAnimation on opacity {
                        running: root.stopEnabled
                        loops: Animation.Infinite
                        NumberAnimation { from: 0.05; to: 0.25; duration: 900; easing.type: Easing.InOutSine }
                        NumberAnimation { from: 0.25; to: 0.05; duration: 900; easing.type: Easing.InOutSine }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: recButton.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    acceptedButtons: Qt.NoButton
                    hoverEnabled: recButton.enabled
                }

                onClicked: {
                    if (root.stopEnabled)
                        root.stopRequested();
                    else
                        root.startRequested();
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: root.waveformHeight
                color: Theme.recessedBackground
                radius: 6
                border.color: Theme.borderDefault
                clip: true

                WaveformView {
                    anchors.fill: parent
                    anchors.margins: 4
                    waveformData: Backend.recordingWaveform
                    layerColor: root.lastPlaybackType === Backend.PlaybackReplay ? Theme.accentGreen : Theme.accentPurple
                    playPositionMs: Backend.playbackPosition
                    readOnly: true
                    emptyText: "No recording yet"
                }

                // Play/Stop Overlay Button (centered on top of the waveform)
                Rectangle {
                    anchors.centerIn: parent
                    width: 40
                    height: 40
                    radius: 20
                    color: playMouseArea.containsMouse ? Qt.rgba(0, 0, 0, 0.75) : Qt.rgba(0, 0, 0, 0.5)
                    border.color: "white"
                    border.width: 1
                    visible: root.playEnabled && !root.stopEnabled

                    Image {
                        anchors.centerIn: parent
                        source: "image://icons/" + (Backend.isPlaying ? "square" : "play") + "?color=%23ffffff"
                        width: 16
                        height: 16
                    }

                    MouseArea {
                        id: playMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (Backend.isPlaying) {
                                Backend.stopPlayback();
                            } else if (root.lastRecordingPath.length > 0) {
                                Backend.playFile(root.lastRecordingPath);
                            }
                        }
                    }
                }
            }
        }
    }
}
