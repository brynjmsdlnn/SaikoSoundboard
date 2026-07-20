import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Rectangle {
    id: root

    property string captureMode: "global"
    property bool isCaptureReady: true
    property string lastRecordingPath: ""
    property int lastPlaybackType: Backend.PlaybackNone
    property bool startEnabled: true
    property bool stopEnabled: false
    property bool playEnabled: false
    property int recordingDurationSec: 10
    property int waveformHeight: 80
    property int cardPadding: 12
    property int pulseDurationMs: 800

    signal startRequested
    signal stopRequested
    signal assignToSlotRequested

    implicitHeight: sectionContent.implicitHeight + 24

    // Dynamic background and border color matching the current state
    color: root.stopEnabled ? Qt.rgba(Theme.accentRed.r, Theme.accentRed.g, Theme.accentRed.b, 0.04) : (Backend.isPlaying ? (root.lastPlaybackType === Backend.PlaybackReplay ? Qt.rgba(Theme.accentGreen.r, Theme.accentGreen.g, Theme.accentGreen.b, 0.04) : Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.04)) : Theme.appBackground)

    border.color: root.stopEnabled ? Theme.accentRed : (Backend.isPlaying ? (root.lastPlaybackType === Backend.PlaybackReplay ? Theme.accentGreen : Theme.accentPurple) : Theme.borderDefault)

    radius: Theme.cardRadius

    Behavior on color {
        ColorAnimation {
            duration: 300
        }
    }
    Behavior on border.color {
        ColorAnimation {
            duration: 300
        }
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
                    NumberAnimation {
                        to: 0.3
                        duration: root.pulseDurationMs
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        to: 1.0
                        duration: root.pulseDurationMs
                        easing.type: Easing.InOutQuad
                    }
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

            // Pushes everything after this point (Info Text & Assign to Slot) to the right side
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

            // Assign to Slot
            Rectangle {
                id: assignToSlotContainer
                Layout.preferredWidth: assignToSlotRow.implicitWidth
                Layout.fillHeight: true
                radius: 6
                visible: root.playEnabled && !root.stopEnabled

                color: assignToSlotRow.isHovered ? "#1a1a1a" : "transparent"
                border.color: assignToSlotRow.isHovered ? Theme.borderHover : "transparent"
                border.width: 1

                Behavior on color {
                    ColorAnimation {
                        duration: 150
                    }
                }
                Behavior on border.color {
                    ColorAnimation {
                        duration: 150
                    }
                }

                RowLayout {
                    id: assignToSlotRow
                    anchors.fill: parent
                    anchors.leftMargin: 2
                    anchors.rightMargin: 2
                    spacing: 0

                    property bool isHovered: assignToSlotIconArea.containsMouse || assignToSlotLabelArea.containsMouse
                    property bool isPressed: assignToSlotIconArea.pressed || assignToSlotLabelArea.pressed

                    scale: assignToSlotRow.isPressed ? 0.8 : 1.0

                    Behavior on scale {
                        NumberAnimation {
                            duration: 120
                            easing.type: Easing.OutBack
                        }
                    }

                    // 1. ICON FIRST
                    Item {
                        id: assignToSlotIcon
                        implicitWidth: 22
                        Layout.fillHeight: true

                        Image {
                            id: assignToSlotIconImage
                            anchors.centerIn: parent
                            sourceSize: Qt.size(14, 14)
                            source: "image://icons/grid-2x2-plus?color=%23ffffff"
                        }

                        MouseArea {
                            id: assignToSlotIconArea
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: root.assignToSlotRequested()
                        }
                    }

                    // 2. TEXT SECOND: Slides right
                    Item {
                        id: assignToSlotLabel
                        Layout.fillHeight: true
                        Layout.preferredWidth: assignToSlotRow.isHovered ? (assignToSlotText.implicitWidth + 8) : 0
                        opacity: assignToSlotRow.isHovered ? 1.0 : 0.0
                        visible: opacity > 0.0
                        clip: true

                        Behavior on Layout.preferredWidth {
                            NumberAnimation {
                                duration: 250
                                easing.type: Easing.InOutQuad
                            }
                        }
                        Behavior on opacity {
                            NumberAnimation {
                                duration: 200
                                easing.type: Easing.InOutQuad
                            }
                        }

                        MouseArea {
                            id: assignToSlotLabelArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.assignToSlotRequested()
                        }

                        Text {
                            id: assignToSlotText
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.verticalCenterOffset: -0.6

                            text: "Assign to Slot"
                            color: Theme.textSecondary
                            font.bold: true
                            font.pixelSize: 12
                        }
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
                enabled: (root.startEnabled || root.stopEnabled) && root.isCaptureReady
                opacity: root.isCaptureReady ? (enabled ? 1.0 : 0.4) : 0.65

                Behavior on opacity {
                    NumberAnimation {
                        duration: 150
                    }
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
                        NumberAnimation {
                            from: 0.05
                            to: 0.25
                            duration: 900
                            easing.type: Easing.InOutSine
                        }
                        NumberAnimation {
                            from: 0.25
                            to: 0.05
                            duration: 900
                            easing.type: Easing.InOutSine
                        }
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
                id: waveformContainer
                Layout.fillWidth: true
                Layout.preferredHeight: root.waveformHeight
                color: Theme.recessedBackground
                radius: 6
                border.color: Theme.borderDefault
                clip: true

                HoverHandler {
                    id: waveformHover
                }

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
                    id: playOverlayButton
                    anchors.centerIn: parent
                    width: 40
                    height: 40
                    radius: 20
                    
                    // Scale transitions
                    scale: playMouseArea.containsMouse ? 1.1 : 1.0
                    
                    // Visibility transitions
                    opacity: (root.playEnabled && !root.stopEnabled && waveformHover.hovered) ? 1.0 : 0.0
                    visible: opacity > 0.0

                    // Dynamic colors based on hover
                    color: playMouseArea.containsMouse ? 
                           Qt.rgba(buttonThemeColor.r, buttonThemeColor.g, buttonThemeColor.b, 0.2) : 
                           Qt.rgba(30/255, 30/255, 35/255, 0.9)
                           
                    border.color: playMouseArea.containsMouse ? buttonThemeColor : "white"
                    border.width: 1

                    // Define the theme color helper property (green for play, red for stop)
                    readonly property color buttonThemeColor: Backend.isPlaying ? Theme.accentRed : Theme.accentGreen

                    Behavior on scale { NumberAnimation { duration: 250; easing.type: Easing.OutBack } }
                    Behavior on opacity { NumberAnimation { duration: 150 } }
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Behavior on border.color { ColorAnimation { duration: 150 } }

                    Image {
                        anchors.centerIn: parent
                        source: "image://icons/" + (Backend.isPlaying ? "square" : "play") + "?color=" + (playMouseArea.containsMouse ? encodeURIComponent(playOverlayButton.buttonThemeColor) : "%23ffffff")
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
