import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0
import "../shared/utils.js" as Utils

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

    implicitHeight: sectionContent.implicitHeight + 24
    color: Theme.appBackground
    radius: Theme.cardRadius
    border.color: Theme.borderDefault

    ColumnLayout {
        id: sectionContent
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: root.cardPadding
        spacing: 8

        property bool isReplayActive: Backend.recording.isReplayActive

        // Row 1: Header (Full Width)
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                id: statusDot
                width: 8
                height: 8
                radius: 4
                color: sectionContent.isReplayActive ? Theme.accentGreen : Theme.textDim
                opacity: 1.0

                SequentialAnimation on opacity {
                    running: sectionContent.isReplayActive
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.3; duration: root.pulseDurationMs; easing.type: Easing.InOutQuad }
                    NumberAnimation { to: 1.0; duration: root.pulseDurationMs; easing.type: Easing.InOutQuad }
                }

                Connections {
                    target: sectionContent
                    function onIsReplayActiveChanged() {
                        if (!sectionContent.isReplayActive)
                            statusDot.opacity = 1.0;
                    }
                }
            }

            Text {
                text: "Replay Buffer"
                color: Theme.textPrimary
                font.bold: true
            }

            SaikoIconButton {
                iconSource: "image://icons/folder?color=%23b0b0b0"
                tooltipText: "Open Replays Folder"
                onClicked: Qt.openUrlExternally("file:///" + encodeURI(Backend.settings.replayDirectory))
            }

            Item {
                Layout.fillWidth: true
            }

            Text {
                text: "Duration:"
                color: Theme.textSecondary
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
        }

        // Row 2: Controls and Waveform inline side-by-side
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            // Left: Big Save Replay Button (styled like Soundboard Slot Editor action buttons)
            Button {
                id: saveButton
                implicitWidth: 80
                Layout.preferredHeight: 80
                enabled: root.saveReplayEnabled && sectionContent.isReplayActive && root.replayChecked
                opacity: enabled ? 1.0 : 0.4

                Behavior on opacity {
                    NumberAnimation { duration: 150 }
                }
                
                background: Rectangle {
                    color: parent.enabled && parent.hovered ? Theme.inputBackground : Theme.recessedBackground
                    radius: 8
                    border.color: parent.enabled && parent.hovered ? Theme.accentPurple : Theme.borderDefault
                    border.width: 1
                }

                contentItem: ColumnLayout {
                    spacing: 6
                    Layout.alignment: Qt.AlignVCenter

                    Image {
                        source: "image://icons/save?color=%23" + (saveButton.enabled ? "bb86fc" : "b0b0b0")
                        sourceSize: Qt.size(24, 24)
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: "Save"
                        color: Theme.textPrimary
                        font.pixelSize: 12
                        font.weight: Font.Medium
                        Layout.alignment: Qt.AlignHCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: saveButton.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    acceptedButtons: Qt.NoButton
                    hoverEnabled: saveButton.enabled
                }

                onClicked: {
                    if (!sectionContent.isReplayActive)
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
                    waveformData: Backend.replayWaveform
                    layerColor: Theme.accentGreen
                    readOnly: true
                    emptyText: "Replay buffer empty"
                    opacity: root.replayChecked ? 1.0 : 0.15
                }

                // Disabled Cover Overlay (with centered Enable button)
                Rectangle {
                    anchors.fill: parent
                    color: Qt.rgba(0, 0, 0, 0.4)
                    visible: !root.replayChecked

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 12

                        Text {
                            text: "Replay Buffer Disabled"
                            color: Theme.textSecondary
                            font.pixelSize: 12
                            font.weight: Font.Medium
                        }

                        SaikoButton {
                            text: "Enable"
                            small: true
                            onClicked: {
                                Backend.settings.replayEnabled = true;
                                Backend.settings.save();
                                Backend.recording.setReplayEnabled(true, root.captureMode);
                            }
                        }
                    }
                }

                // Top-right Disable Overlay Button (visible only when enabled)
                SaikoButton {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: 4
                    text: "Disable"
                    small: true
                    implicitWidth: 60
                    visible: root.replayChecked
                    onClicked: {
                        Backend.settings.replayEnabled = false;
                        Backend.settings.save();
                        Backend.recording.setReplayEnabled(false, root.captureMode);
                    }
                }
            }
        }
    }
}
