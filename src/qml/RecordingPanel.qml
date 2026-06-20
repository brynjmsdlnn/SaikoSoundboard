import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0
import "utils.js" as Utils

Flickable {
    id: root

    property string captureMode: "global"
    property string lastRecordingPath: ""

    signal startRequested()
    signal stopRequested()
    signal changeFolderRequested()
    signal captureModeSelected(string newMode)

    SplitView.minimumWidth: 300
    contentWidth: width
    contentHeight: leftColumn.implicitHeight + 32
    clip: true

    property bool startEnabled: true
    property bool stopEnabled: false
    property bool playEnabled: false
    property bool modeEnabled: true
    property bool saveReplayEnabled: false
    property bool replayChecked: false
    property string statusText: "Ready"
    property string replayStatusText: "Status: inactive"

    property int __remainingSec: 0
    property real __elapsedSec: 0.0

    function notifyRecordingStarted() {
        root.startEnabled = false
        root.stopEnabled = true
        root.playEnabled = false
        __remainingSec = 10
        __elapsedSec = 0.0
        timerLabel.text = "Time remaining: 10s"
        statsLabel.text = "Size: 0 KB \u00b7 Time: 0.0s"
        recordingTimer.start()
        stopTimer.start()
    }

    function notifyRecordingStopped() {
        recordingTimer.stop()
        stopTimer.stop()
    }

    function setStatusText(text) {
        root.statusText = text
    }

    function setPlayEnabled(enabled) {
        root.playEnabled = enabled
    }

    function setStartEnabled(enabled) {
        root.startEnabled = enabled
    }

    function setModeEnabled(enabled) {
        root.modeEnabled = enabled
    }

    function setReplayChecked(checked) {
        root.replayChecked = checked
    }

    function setSaveReplayEnabled(enabled) {
        root.saveReplayEnabled = enabled
    }

    function setReplayStatusText(text) {
        root.replayStatusText = text
    }

    function resetUI() {
        timerLabel.text = ""
        root.stopEnabled = false
        root.startEnabled = true
    }

    Timer {
        id: recordingTimer
        interval: 100
        repeat: true
        onTriggered: {
            root.__elapsedSec += 0.1
            root.__remainingSec = Math.max(0, 10 - Math.floor(root.__elapsedSec))
            timerLabel.text = "Time remaining: " + root.__remainingSec + "s"
            var bytes = Backend.recordingFileSize()
            statsLabel.text = "Size: " + Math.round(bytes/1024) + " KB \u00b7 Time: " + root.__elapsedSec.toFixed(1) + "s"
        }
    }

    Timer {
        id: stopTimer
        interval: 10000
        repeat: false
        onTriggered: root.stopRequested()
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

        SectionCard {
            heading: "STATUS"

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    id: statusLabel
                    text: root.statusText
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

        SectionCard {
            heading: "CAPTURE"

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

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
                            enabled: root.modeEnabled
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
                                root.captureMode = modeCombo.currentValue
                                root.captureModeSelected(modeCombo.currentValue)
                            }
                        }
                    }
                }

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
                    ThemedButton { text: "Change..."; small: true; onClicked: root.changeFolderRequested() }
                }
            }
        }

        SectionCard {
            heading: "REPLAY BUFFER"

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    CustomCheckBox {
                        id: replayCheck
                        text: "Enabled"
                        checked: root.replayChecked
                        onToggled: {
                            Backend.settings.replayEnabled = checked
                            Backend.settings.save()
                            Backend.recording.setReplayEnabled(checked, root.captureMode)
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
                        text: root.replayStatusText
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSizeSmall
                    }
                    Item { Layout.fillWidth: true }
                    ThemedButton {
                        id: saveReplayBtn
                        text: "Save replay"
                        small: true
                        enabled: root.saveReplayEnabled
                        onClicked: {
                            if (Backend.recording.isReplayActive) {
                                var fmt = Utils.formatTimestamp(new Date())
                                var path = Backend.settings.saveDirectory + "/Replay_" + fmt + ".wav"
                                if (Backend.recording.saveReplay(path)) {
                                    root.lastRecordingPath = path
                                    root.setStatusText("Replay saved: Replay_" + fmt + ".wav")
                                    root.playEnabled = true
                                } else {
                                    root.setStatusText("Failed to save replay or buffer empty")
                                }
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            ThemedButton {
                id: startBtn
                text: "Start recording"
                accentColor: Theme.accentGreen
                filled: true
                enabled: root.startEnabled
                onClicked: root.startRequested()
            }
            ThemedButton {
                id: stopBtn
                text: "Stop"
                accentColor: Theme.accentRed
                filled: true
                enabled: root.stopEnabled
                onClicked: root.stopRequested()
            }
            ThemedButton {
                id: playBtn
                text: "Play last"
                accentColor: Theme.accentPurple
                filled: true
                enabled: root.playEnabled
                onClicked: {
                    if (root.lastRecordingPath.length > 0) Backend.playFile(root.lastRecordingPath)
                }
            }
        }

        Item { Layout.preferredHeight: 8 }
    }
}
