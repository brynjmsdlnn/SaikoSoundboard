import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

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

            // ── Replay duration: display ↔ edit with cross-fade transition ──
            ReplayDurationEditor {
                id: durationWidget
                Layout.alignment: Qt.AlignRight
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
                    NumberAnimation {
                        duration: 150
                    }
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
                    var path = Backend.generateReplayFilePath();
                    if (Backend.recording.saveReplay(path)) {
                        root.replaySaved(path);
                        root.statusMessage("Replay saved: " + path.substring(path.lastIndexOf("/") + 1));
                    } else {
                        root.statusMessage("Failed to save replay");
                    }
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
                    id: containerHover
                }

                WaveformView {
                    anchors.fill: parent
                    anchors.margins: 4
                    waveformData: Backend.replayWaveform
                    layerColor: Theme.accentGreen
                    readOnly: true
                    emptyText: "Replay buffer empty"

                    // Smoothly fade the waveform view in and out
                    opacity: root.replayChecked ? 1.0 : 0.15
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 350
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                // One-Shot Shockwave Animation Overlay
                Canvas {
                    id: waveCanvas
                    anchors.fill: parent

                    property real waveProgress: 0.0
                    property bool playing: false

                    visible: playing
                    opacity: playing ? 1.0 : 0.0

                    onWaveProgressChanged: requestPaint()

                    function emitShockwave() {
                        fadeOut.stop()
                        waveProgress = 0.0
                        opacity = 1.0
                        playing = true
                        shockwave.restart()
                    }

                    function endShockwave() {
                        if (playing) {
                            fadeOut.restart()
                        }
                    }

                    NumberAnimation {
                        id: shockwave

                        target: waveCanvas
                        property: "waveProgress"

                        from: 0.0
                        to: 1.0

                        duration: 5000
                        easing.type: Easing.OutQuart
                    }

                    NumberAnimation {
                        id: fadeOut

                        target: waveCanvas
                        property: "opacity"

                        to: 0.0

                        duration: 350

                        onFinished: {
                            waveCanvas.playing = false
                            shockwave.stop()
                        }
                    }

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.reset();

                        var cx = powerButton.x + powerButton.width / 2;
                        var cy = powerButton.y + powerButton.height / 2;

                        drawWave(ctx, cx, cy, waveProgress);
                    }

                    function drawWave(ctx, cx, cy, progress) {

                        // Changed to match the enabled button state (Red) instead of Green
                        var clr = Theme.accentRed;

                        var r = Math.round(clr.r * 255);
                        var g = Math.round(clr.g * 255);
                        var b = Math.round(clr.b * 255);

                        var startRadius = 4;
                        var maxRadius = Math.sqrt(width * width + height * height);
                        var radius = startRadius + (maxRadius - startRadius) * progress;

                        var alpha = Math.pow(1.0 - progress, 1.2);

                        ctx.save();

                        //----------------------------------------------------
                        // PASS 0 : Dim the part it passes through
                        //----------------------------------------------------
                        ctx.beginPath();
                        ctx.arc(cx, cy, radius, 0, Math.PI * 2);
                        ctx.fillStyle = "rgba(0, 0, 0, 0.65)";
                        ctx.fill();

                        //----------------------------------------------------
                        // PASS 1 : Vignette
                        //----------------------------------------------------
                        var vignette = ctx.createRadialGradient(
                                    cx, cy, radius * 0.45,
                                    cx, cy, radius * 1.25);

                        vignette.addColorStop(0.0, "rgba(0,0,0,0)");
                        vignette.addColorStop(0.55, "rgba(0,0,0,0)");
                        vignette.addColorStop(0.82, "rgba(0,0,0," + (alpha * 0.12) + ")");
                        vignette.addColorStop(1.0, "rgba(0,0,0," + (alpha * 0.40) + ")");

                        ctx.fillStyle = vignette;

                        ctx.beginPath();
                        ctx.arc(cx, cy, radius * 1.25, 0, Math.PI * 2);
                        ctx.fill();

                        // (The thick "Large Glow" pass was removed entirely here to eliminate the heavy glow)

                        //----------------------------------------------------
                        // PASS 2 : Sharp bright edge ring
                        //----------------------------------------------------
                        ctx.beginPath();
                        ctx.arc(cx, cy, radius, 0, Math.PI * 2);

                        ctx.strokeStyle =
                                "rgba(" +
                                r + "," +
                                g + "," +
                                b + "," +
                                (alpha * 0.95) + ")";

                        ctx.lineWidth = 2;
                        ctx.stroke();

                        //----------------------------------------------------
                        // PASS 3 : Center flash
                        //----------------------------------------------------
                        var center = ctx.createRadialGradient(
                                    cx,
                                    cy,
                                    0,
                                    cx,
                                    cy,
                                    40);

                        center.addColorStop(
                                    0,
                                    "rgba(" +
                                    r + "," +
                                    g + "," +
                                    b + "," +
                                    (alpha * 0.30) + ")");

                        center.addColorStop(
                                    1,
                                    "rgba(" +
                                    r + "," +
                                    g + "," +
                                    b + ",0)");

                        ctx.fillStyle = center;

                        ctx.beginPath();
                        ctx.arc(cx, cy, 40, 0, Math.PI * 2);
                        ctx.fill();

                        ctx.restore();
                    }

                    onWidthChanged: requestPaint()
                    onHeightChanged: requestPaint()
                }

                // Disabled Cover Overlay (Glassmorphic Fade)
                Rectangle {
                    anchors.fill: parent
                    color: Qt.rgba(13 / 255, 13 / 255, 15 / 255, 0.85)

                    // Backdrop dims to 0.85 on hover when disabled, stays at 0.3 base when disabled & not hovered, and is 0.0 when enabled
                    opacity: root.replayChecked ? 0.0 : (containerHover.hovered ? 1.0 : 0.3)
                    visible: opacity > 0.0

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 350
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                // Centered Circular Power Button (Acts as toggle)
                Rectangle {
                    id: powerButton
                    anchors.centerIn: parent
                    width: 40
                    height: 40
                    radius: 20

                    // When enabled: only show on waveform hover. When disabled: always visible.
                    opacity: root.replayChecked ?
                             (containerHover.hovered ? 1.0 : 0.0) :
                             (btnMouse.containsMouse ? 1.0 : (containerHover.hovered ? 0.8 : 0.5))

                    visible: opacity > 0.0
                    scale: btnMouse.containsMouse ? 1.1 : 1.0

                    color: root.replayChecked ?
                           (btnMouse.containsMouse ? Qt.rgba(Theme.accentRed.r, Theme.accentRed.g, Theme.accentRed.b, 0.2) : Qt.rgba(30/255, 30/255, 35/255, 0.9)) :
                           (btnMouse.containsMouse ? Qt.rgba(Theme.accentGreen.r, Theme.accentGreen.g, Theme.accentGreen.b, 0.2) : Qt.rgba(30/255, 30/255, 35/255, 0.9))

                    border.color: root.replayChecked ?
                                  (btnMouse.containsMouse ? Theme.accentRed : Theme.accentGreen) :
                                  (btnMouse.containsMouse ? Theme.accentGreen : "white")
                    border.width: 1

                    Behavior on scale { NumberAnimation { duration: 250; easing.type: Easing.OutBack } }
                    Behavior on opacity { NumberAnimation { duration: 200 } }
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Behavior on border.color { ColorAnimation { duration: 150 } }

                    // Pulse Ring effect (green when disabled, red when enabled & hovered)
                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: -4
                        radius: 24
                        color: "transparent"
                        border.color: root.replayChecked ? Theme.accentRed : Theme.accentGreen
                        border.width: 1.5
                        opacity: btnMouse.containsMouse ? 0.6 : (root.replayChecked ? 0.0 : 0.2)
                        scale: btnMouse.containsMouse ? 1.1 : 1.0

                        Behavior on opacity { NumberAnimation { duration: 200 } }
                        Behavior on scale { NumberAnimation { duration: 200 } }
                    }

                    Image {
                        anchors.centerIn: parent
                        source: "image://icons/power?color=" + (root.replayChecked ? (btnMouse.containsMouse ? encodeURIComponent(Theme.accentRed) : encodeURIComponent(Theme.accentGreen)) : (btnMouse.containsMouse ? encodeURIComponent(Theme.accentGreen) : "%23ffffff"))
                        sourceSize: Qt.size(16, 16)
                    }

                    MouseArea {
                        id: btnMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onEntered: {
                            // Only trigger the shockwave when replay is enabled
                            if (root.replayChecked) {
                                waveCanvas.emitShockwave()
                            }
                        }

                        onExited: {
                            waveCanvas.endShockwave()
                        }

                        onClicked: {
                            var nextState = !root.replayChecked;

                            Backend.settings.replayEnabled = nextState;
                            Backend.settings.save();
                            Backend.recording.setReplayEnabled(nextState, root.captureMode);
                        }
                    }
                }
            }
        }
    }
}
