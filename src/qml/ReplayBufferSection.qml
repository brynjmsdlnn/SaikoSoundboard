import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0
import "utils.js" as Utils

Rectangle {
    id: root

    property string captureMode: "global"
    property bool replayChecked: false
    property bool saveReplayEnabled: false
    property int pulseDurationMs: 800
    property int waveformHeight: 80
    property int cardPadding: 12

    signal replaySaved(string path)
    signal statusMessage(string text)

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

        property bool isReplayActive: Backend.recording.isReplayActive

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                id: statusDot
                width: 8
                height: 8
                radius: 4
                color: replayContent.isReplayActive ? Theme.accentGreen : Theme.textDim
                opacity: 1.0

                SequentialAnimation on opacity {
                    running: replayContent.isReplayActive
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.3; duration: root.pulseDurationMs; easing.type: Easing.InOutQuad }
                    NumberAnimation { to: 1.0; duration: root.pulseDurationMs; easing.type: Easing.InOutQuad }
                }

                Connections {
                    target: replayContent
                    function onIsReplayActiveChanged() {
                        if (!replayContent.isReplayActive)
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

                HoverHandler { id: hoverHandler }

                SpinBox {
                    id: replayDurationSpin
                    anchors.fill: parent
                    from: 1
                    to: 120
                    value: Backend.settings.replayDuration
                    editable: true
                    background: Item {}
                    textFromValue: function (value) { return value + "s"; }
                    valueFromText: function (text) {
                        var parsed = parseInt(text.replace("s", ""), 10);
                        return isNaN(parsed) ? replayDurationSpin.value : parsed;
                    }
                    down.indicator: Item {
                        x: 0; y: 0; width: 20; height: parent.height
                        opacity: hoverHandler.hovered ? 1.0 : 0.0
                        Text {
                            text: "-"
                            anchors.centerIn: parent
                            color: replayDurationSpin.down.pressed ? Theme.borderDefault : Theme.textPrimary
                        }
                    }
                    up.indicator: Item {
                        x: parent.width - width; y: 0; width: 20; height: parent.height
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
                        validator: IntValidator { bottom: 1; top: 120 }
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
                    if (!replayContent.isReplayActive)
                        return;
                    var stamp = Utils.formatTimestamp(new Date());
                    var path = Backend.settings.replayDirectory + "/Replay_" + stamp + ".wav";
                    if (Backend.recording.saveReplay(path)) {
                        root.replaySaved(path);
                        root.statusMessage("Replay saved: Replay_" + stamp + ".wav");
                    } else {
                        root.statusMessage("Failed to save replay");
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
