import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0
import "utils.js" as Utils

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
        __remainingSec = recordingDurationSec;
        __elapsedSec = 0.0;
        timerLabel.text = "Time remaining: " + recordingDurationSec + "s";
        statsLabel.text = "Size: 0 KB \u00b7 Time: 0.0s";
        recordingTimer.start();
        stopTimer.start();
    }
    function notifyRecordingStopped() {
        recordingTimer.stop();
        stopTimer.stop();
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
        timerLabel.text = "";
        statsLabel.text = "";
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
            root.__elapsedSec += 0.1;
            root.__remainingSec = Math.max(0, root.recordingDurationSec - Math.floor(root.__elapsedSec));
            timerLabel.text = "Time remaining: " + root.__remainingSec + "s";
            var bytes = Backend.recordingFileSize();
            statsLabel.text = "Size: " + Math.round(bytes / 1024) + " KB \u00b7 Time: " + root.__elapsedSec.toFixed(1) + "s";
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
            var pos = Backend.playbackPosition();
            var dur = Backend.playbackDuration;
            timerLabel.text = (pos / 1000).toFixed(1) + "s / " + (dur / 1000).toFixed(1) + "s";
        }
    }
    Connections {
        target: Backend
        function onIsPlayingChanged() {
            if (Backend.isPlaying)
                playbackTimer.start();
            else {
                playbackTimer.stop();
                if (timerLabel.text !== "")
                    timerLabel.text = "";
            }
        }
    }

    ColumnLayout {
        id: mainColumn
        width: parent.width - 16
        x: root.contentMargin
        y: root.contentMargin
        spacing: root.sectionSpacing

        // HEADER - Centered Focus + Stats Dropdown Extension
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: headerColumn.implicitHeight
            radius: Theme.borderRadius
            border.color: Theme.borderDefault
            // We can remove clip: true here, as we will handle the corners perfectly inside

            // Subtle purple tint when playing, otherwise normal background
            color: Backend.isPlaying ? Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.05) : Theme.appBackground
            Behavior on color {
                ColorAnimation {
                    duration: 300
                }
            }

            ColumnLayout {
                id: headerColumn
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: 0

                // ---- TOP ROW (Main Header) ----
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.headerHeight

                    // Left aligned icon (Pulsing & Color changing)
                    Text {
                        id: headerIcon
                        anchors.left: parent.left
                        anchors.leftMargin: root.cardPadding
                        anchors.verticalCenter: parent.verticalCenter
                        text: "((•))"

                        color: Backend.isPlaying ? Theme.accentPurple : Theme.accentGreen
                        font.pixelSize: 16
                        font.bold: true

                        property bool isActive: Backend.isPlaying || !root.startEnabled

                        SequentialAnimation on opacity {
                            running: headerIcon.isActive
                            loops: Animation.Infinite
                            NumberAnimation {
                                to: 0.2
                                duration: 800
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                to: 1.0
                                duration: 800
                                easing.type: Easing.InOutQuad
                            }
                        }

                        onIsActiveChanged: {
                            if (!isActive)
                                opacity = 1.0;
                        }
                    }

                    // Perfectly Centered Status Text
                    Text {
                        anchors.centerIn: parent
                        text: Backend.isPlaying ? "Playing: " + root.lastRecordingPath.split("/").pop() : root.statusText
                        color: Theme.textPrimary
                        font.pixelSize: 14
                        font.weight: Font.Medium
                    }

                    // Right aligned Timer
                    Text {
                        id: timerLabel
                        anchors.right: parent.right
                        anchors.rightMargin: root.cardPadding
                        anchors.verticalCenter: parent.verticalCenter
                        text: ""
                        color: Theme.accentPurple
                        font.pixelSize: 13
                        visible: text !== ""
                    }
                }

                // ---- BOTTOM EXTENSION (Stats Only) ----
                Item {
                    id: statsExtension
                    Layout.fillWidth: true

                    property bool showStats: statsLabel.text !== "" && !Backend.isPlaying

                    Layout.preferredHeight: showStats ? 28 : 0
                    opacity: showStats ? 1.0 : 0.0
                    clip: true

                    Behavior on Layout.preferredHeight {
                        NumberAnimation {
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                    }
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 200
                        }
                    }

                    // Extension Background Structure
                    Rectangle {
                        anchors.fill: parent
                        // Inset by 1px so we don't cover the parent's outer border
                        anchors.leftMargin: 1
                        anchors.rightMargin: 1
                        anchors.bottomMargin: 1

                        color: Theme.recessedBackground

                        // Round all corners (match parent, minus 1px for the inset)
                        radius: Math.max(0, Theme.borderRadius - 1)

                        // "Square off" the top corners by drawing a sharp rectangle over the top half
                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            height: Theme.borderRadius // Only cover the top
                            color: Theme.recessedBackground
                        }

                        // Top separator line
                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            height: 1
                            color: Theme.borderDefault
                        }

                        // Centered Stats Label
                        Text {
                            id: statsLabel
                            anchors.centerIn: parent
                            text: ""
                            color: Theme.textSecondary
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }

        // CAPTURE MODE
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: captureContent.implicitHeight + 24
            color: Theme.appBackground
            radius: Theme.cardRadius
            border.color: Theme.borderDefault
            RowLayout {
                id: captureContent
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: root.cardPadding
                spacing: 10
                Text {
                    text: "Capture Mode:"
                    color: Theme.textSecondary
                    font.bold: true
                }
                CustomComboBox {
                    Layout.preferredWidth: 220
                    model: [
                        {
                            text: "System Output (Global)",
                            value: "global"
                        },
                        {
                            text: "Multi-track (sources)",
                            value: "multi"
                        }
                    ]
                    textRole: "text"
                    valueRole: "value"
                    isActive: root.modeEnabled
                    onActivated: {
                        root.captureMode = currentValue;
                        root.captureModeSelected(currentValue);
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
                ThemedButton {
                    text: "📂 Recordings"
                    small: true
                    onClicked: Qt.openUrlExternally("file:///" + encodeURI(Backend.settings.recordingDirectory))
                }
                ThemedButton {
                    text: "📂 Replays"
                    small: true
                    onClicked: Qt.openUrlExternally("file:///" + encodeURI(Backend.settings.replayDirectory))
                }
                ThemedButton {
                    text: "⚙"
                    small: true
                    implicitWidth: 28
                    onClicked: root.settingsRequested()
                }
            }
        }

        // REPLAY BUFFER
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: replayContent.implicitHeight + 24
            color: Theme.appBackground
            radius: Theme.cardRadius
            border.color: Theme.borderDefault
            ColumnLayout {
                id: replayContent
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: root.cardPadding
                spacing: 8
                property bool isActive: Backend.recording.isReplayActive

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Rectangle {
                        id: statusDot
                        width: 8
                        height: 8
                        radius: 4
                        color: replayContent.isActive ? Theme.accentGreen : Theme.textDim
                        opacity: 1.0
                        SequentialAnimation on opacity {
                            running: replayContent.isActive
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
                            target: replayContent
                            function onIsActiveChanged() {
                                if (!replayContent.isActive)
                                    statusDot.opacity = 1.0;
                            }
                        }
                    }
                    Text {
                        text: "Replay Buffer"
                        color: Theme.textPrimary
                        font.bold: true
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    CustomCheckBox {
                        text: "Enable"
                        checked: root.replayChecked
                        onToggled: {
                            Backend.settings.replayEnabled = checked;
                            Backend.settings.save();
                            Backend.recording.setReplayEnabled(checked, root.captureMode);
                        }
                    }
                    Text {
                        text: "Duration:"
                        color: Theme.textSecondary
                        Layout.leftMargin: 8
                    }
                    Rectangle {
                        width: 70
                        height: 26
                        color: Theme.appBackground
                        radius: 4
                        border.color: Theme.borderDefault
                        HoverHandler {
                            id: hoverHandler
                        }
                        SpinBox {
                            id: replayDurationSpin
                            anchors.fill: parent
                            from: 1
                            to: 120
                            value: Backend.settings.replayDuration
                            editable: true
                            background: Item {}
                            textFromValue: function (value) {
                                return value + "s";
                            }
                            valueFromText: function (text) {
                                var parsed = parseInt(text.replace("s", ""), 10);
                                return isNaN(parsed) ? replayDurationSpin.value : parsed;
                            }
                            down.indicator: Item {
                                x: 0
                                y: 0
                                width: 20
                                height: parent.height
                                opacity: hoverHandler.hovered ? 1.0 : 0.0
                                Text {
                                    text: "-"
                                    anchors.centerIn: parent
                                    color: replayDurationSpin.down.pressed ? Theme.borderDefault : Theme.textPrimary
                                }
                            }
                            up.indicator: Item {
                                x: parent.width - width
                                y: 0
                                width: 20
                                height: parent.height
                                opacity: hoverHandler.hovered ? 1.0 : 0.0
                                Text {
                                    text: "+"
                                    anchors.centerIn: parent
                                    color: replayDurationSpin.up.pressed ? Theme.borderDefault : Theme.textPrimary
                                }
                            }
                            contentItem: TextInput {
                                text: replayDurationSpin.textFromValue(replayDurationSpin.value, replayDurationSpin.locale)
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignVCenter
                                color: Theme.textPrimary
                                selectionColor: Theme.borderDefault
                                selectByMouse: true
                                readOnly: !replayDurationSpin.editable
                                validator: IntValidator {
                                    bottom: 1
                                    top: 120
                                }
                            }
                            onValueModified: {
                                Backend.recording.setReplayDuration(value);
                                Backend.settings.replayDuration = value;
                                Backend.settings.save();
                            }
                        }
                    }
                    ThemedButton {
                        text: "Save Replay"
                        small: true
                        enabled: root.saveReplayEnabled
                        Layout.leftMargin: 8
                        onClicked: {
                            if (!replayContent.isActive)
                                return;
                            var stamp = Utils.formatTimestamp(new Date());
                            var path = Backend.settings.replayDirectory + "/Replay_" + stamp + ".wav";
                            if (Backend.recording.saveReplay(path)) {
                                root.replaySaved(path);
                                root.setStatusText("Replay saved: Replay_" + stamp + ".wav");
                                root.playEnabled = true;
                            } else {
                                root.setStatusText("Failed to save replay");
                            }
                        }
                    }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.waveformHeight
                    color: Theme.recessedBackground
                    radius: 6
                    border.color: Theme.borderDefault
                    WaveformView {
                        anchors.fill: parent
                        anchors.margins: 4
                        waveformData: Backend.replayWaveform
                        readOnly: true
                    }
                }
            }
        }

        // TRANSPORT CONTROLS
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
                            {
                                label: "● Rec",
                                enabled: root.startEnabled,
                                accent: Theme.accentRed,
                                fontSize: 14,
                                action: "start"
                            },
                            {
                                label: "■ Stop",
                                enabled: root.stopEnabled,
                                accent: Theme.borderDefault,
                                fontSize: 14,
                                action: "stop"
                            },
                            {
                                label: "▶ Play Recording",
                                enabled: root.playEnabled,
                                accent: Theme.borderDefault,
                                fontSize: 11,
                                action: "play"
                            }
                        ]
                        delegate: Rectangle {
                            required property var modelData
                            readonly property bool isRecButton: modelData.action === "start"
                            width: root.controlButtonWidth
                            height: root.controlButtonHeight
                            radius: 4
                            opacity: modelData.enabled ? 1.0 : 0.5
                            border.color: isRecButton ? Theme.accentRed : Theme.borderDefault
                            color: {
                                if (!modelData.enabled)
                                    return "transparent";
                                if (mouseArea.pressed)
                                    return isRecButton ? Theme.accentRed : Theme.borderDefault;
                                if (mouseArea.hovered)
                                    return isRecButton ? Qt.rgba(Theme.accentRed.r, Theme.accentRed.g, Theme.accentRed.b, 0.1) : Theme.recessedBackground;
                                return Theme.appBackground;
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
                                    if (!modelData.enabled)
                                        return;
                                    switch (modelData.action) {
                                    case "start":
                                        root.startRequested();
                                        break;
                                    case "stop":
                                        root.stopRequested();
                                        break;
                                    case "play":
                                        if (root.lastRecordingPath.length > 0)
                                            Backend.playFile(root.lastRecordingPath);
                                        break;
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
