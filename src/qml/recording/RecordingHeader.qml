import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Rectangle {
    id: root

    property string lastRecordingPath: ""
    property string statusText: "Ready"
    property bool startEnabled: true
    property int recordingDurationSec: 10
    property int cardPadding: 12
    property int headerHeight: 38

    signal stopRequested

    implicitHeight: headerColumn.implicitHeight
    radius: Theme.borderRadius
    border.color: Theme.borderDefault
    color: Backend.isPlaying ? Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.05) : Theme.appBackground

    Behavior on color {
        ColorAnimation { duration: 300 }
    }

    property int __remainingSec: 0
    property real __elapsedSec: 0.0

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

    function notifyRecordingStarted() {
        root.__remainingSec = root.recordingDurationSec;
        root.__elapsedSec = 0.0;
        timerLabel.text = "Time remaining: " + root.recordingDurationSec + "s";
        statsLabel.text = "Size: 0 KB \u00b7 Time: 0.0s";
        recordingTimer.start();
        stopTimer.start();
    }
    function notifyRecordingStopped() {
        recordingTimer.stop();
        stopTimer.stop();
    }
    function resetUI() {
        timerLabel.text = "";
        statsLabel.text = "";
    }

    ColumnLayout {
        id: headerColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.headerHeight

            Text {
                id: headerIcon
                anchors.left: parent.left
                anchors.leftMargin: root.cardPadding
                anchors.verticalCenter: parent.verticalCenter
                text: "((\u2022))"

                color: Backend.isPlaying ? Theme.accentPurple : Theme.accentGreen
                font.pixelSize: 16
                font.bold: true

                property bool isActive: Backend.isPlaying || !root.startEnabled

                SequentialAnimation on opacity {
                    running: headerIcon.isActive
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.2; duration: 800; easing.type: Easing.InOutQuad }
                    NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
                }

                onIsActiveChanged: {
                    if (!isActive)
                        opacity = 1.0;
                }
            }

            Text {
                anchors.centerIn: parent
                text: Backend.isPlaying ? "Playing: " + root.lastRecordingPath.split("/").pop() : root.statusText
                color: Theme.textPrimary
                font.pixelSize: 14
                font.weight: Font.Medium
            }

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

        Item {
            id: statsExtension
            Layout.fillWidth: true

            property bool showStats: statsLabel.text !== "" && !Backend.isPlaying

            Layout.preferredHeight: showStats ? 28 : 0
            opacity: showStats ? 1.0 : 0.0
            clip: true

            Behavior on Layout.preferredHeight {
                NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
            }
            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }

            Rectangle {
                anchors.fill: parent
                anchors.leftMargin: 1
                anchors.rightMargin: 1
                anchors.bottomMargin: 1

                color: Theme.recessedBackground
                radius: Math.max(0, Theme.borderRadius - 1)

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: Theme.borderRadius
                    color: Theme.recessedBackground
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: 1
                    color: Theme.borderDefault
                }

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
