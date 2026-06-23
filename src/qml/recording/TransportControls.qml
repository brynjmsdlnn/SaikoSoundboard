import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

ColumnLayout {
    id: root

    property bool startEnabled: true
    property bool stopEnabled: false
    property bool playEnabled: false
    property string lastRecordingPath: ""
    property int controlsRowHeight: 44

    signal startRequested
    signal stopRequested

    spacing: 4

    Item {
        Layout.fillWidth: true
        Layout.preferredHeight: root.controlsRowHeight

        RowLayout {
            anchors.centerIn: parent
            spacing: 16

            Repeater {
                model: [
                    {
                        icon: root.stopEnabled ? "square" : "circle",
                        enabled: root.stopEnabled || root.startEnabled,
                        accent: Theme.accentRed, // Forces the button to stay red when stopped
                        action: root.stopEnabled ? "stop" : "start"
                    },
                    {
                        icon: "play",
                        enabled: root.playEnabled,
                        accent: Theme.textPrimary,
                        action: "play"
                    }
                ]

                delegate: Rectangle {
                    id: buttonControl
                    required property var modelData
                    readonly property bool isRecButton: modelData.action === "start" || modelData.action === "stop"
                    width: 40
                    height: 40
                    radius: 20
                    opacity: modelData.enabled ? 1.0 : 0.3
                    color: "transparent"
                    border.color: mouseArea.containsMouse ? modelData.accent : Theme.borderDefault
                    border.width: mouseArea.containsMouse ? 2 : 1

                    readonly property string infoText: {
                        if (modelData.action === "start")
                            return modelData.enabled ? "Start recording" : "Waiting to start"
                        if (modelData.action === "stop")
                            return modelData.enabled ? "Stop recording" : "Nothing to stop"
                        if (modelData.action === "play")
                            return modelData.enabled ? "Play last recording" : "No recording yet"
                        return ""
                    }

                    // --- RESTORED PULSING BACKGROUND EFFECT ---
                    Rectangle {
                        id: pulseBackground
                        anchors.centerIn: parent
                        width: parent.width
                        height: parent.height
                        radius: parent.radius
                        color: Theme.accentRed
                        visible: modelData.action === "stop" && root.stopEnabled

                        SequentialAnimation on opacity {
                            running: pulseBackground.visible
                            loops: Animation.Infinite
                            NumberAnimation { from: 0.05; to: 0.25; duration: 900; easing.type: Easing.InOutSine }
                            NumberAnimation { from: 0.25; to: 0.05; duration: 900; easing.type: Easing.InOutSine }
                        }

                        SequentialAnimation on scale {
                            running: pulseBackground.visible
                            loops: Animation.Infinite
                            NumberAnimation { from: 0.85; to: 1.25; duration: 900; easing.type: Easing.InOutSine }
                            NumberAnimation { from: 1.25; to: 0.85; duration: 900; easing.type: Easing.InOutSine }
                        }
                    }

                    // --- REFACTORED DIRECTIONAL TOOLTIP ---
                    SaikoTooltip {
                        text: buttonControl.infoText
                        hovered: mouseArea.containsMouse
                        direction: modelData.action === "play" ? "right" : "left"
                    }

                    Image {
                        anchors.centerIn: parent
                        source: "image://icons/" + modelData.icon + "?color=%23" + (modelData.accent === Theme.accentRed ? "e35d5d" : "b0b0b0")
                        width: 16
                        height: 16
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