import QtQuick 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Rectangle {
    id: root
    implicitHeight: 55
    color: Theme.recessedBackground
    radius: Theme.borderRadius
    border.color: Theme.borderDefault
    border.width: 1
    clip: true

    property var waveformData: null

    property string filePath: ""
    property bool fileExists: true
    property string emptyText: "No audio file"

    readonly property bool showPlaceholder: !fileExists || !dataSource.valid || !dataSource.peaks || dataSource.peaks.length === 0

    property alias startMs: dataSource.startMs
    property alias endMs: dataSource.endMs
    property alias playPositionMs: dataSource.playPositionMs
    property alias readOnly: dataSource.readOnly

    signal trimRangeChanged(double startMs, double endMs)
    signal trimRangeCommit(double startMs, double endMs)

    onWaveformDataChanged: {
        dataSource.setWaveformData(waveformData)
    }

    WaveformData {
        id: dataSource
        onTrimRangeChanged: root.trimRangeChanged(startMs, endMs)
        onTrimRangeCommit: root.trimRangeCommit(startMs, endMs)
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        Connections {
            target: dataSource
            function onDataChanged() { canvas.requestPaint() }
            function onStartMsChanged() { canvas.requestPaint() }
            function onEndMsChanged() { canvas.requestPaint() }
            function onPlayPositionMsChanged() { canvas.requestPaint() }
            function onReadOnlyChanged() { canvas.requestPaint() }
        }

        onPaint: {
            var ctx = canvas.getContext("2d")
            var w = canvas.width
            var h = canvas.height
            var labelH = 15
            var graphH = h - labelH
            var centerY = graphH / 2

            var peaks = dataSource.peaks
            var duration = dataSource.durationMs > 0 ? dataSource.durationMs : 1
            var currentEnd = (dataSource.endMs === -1 || dataSource.endMs === 0) ? duration : dataSource.endMs

            ctx.clearRect(0, 0, w, h)

            ctx.fillStyle = Theme.recessedBackground
            ctx.fillRect(0, 0, w, h)

            if (!dataSource.valid || !peaks || peaks.length === 0) {
                return
            }

            var startX = (dataSource.startMs / duration) * w
            var endX = (currentEnd / duration) * w

            // Center reference line
            ctx.save()
            ctx.strokeStyle = Theme.borderDefault
            ctx.lineWidth = 1
            ctx.setLineDash([4, 4])
            ctx.beginPath()
            ctx.moveTo(0, centerY)
            ctx.lineTo(w, centerY)
            ctx.stroke()
            ctx.restore()

            // Active region highlight
            ctx.fillStyle = "rgba(187, 134, 252, 0.08)"
            ctx.fillRect(Math.max(0, startX), 0, Math.max(1, endX - startX), graphH)

            // Waveform peaks
            var numPeaks = peaks.length
            var step = w / numPeaks
            ctx.lineWidth = 2

            for (var i = 0; i < numPeaks; i++) {
                var x = i * step
                var peak = peaks[i]
                var peakH = peak * (graphH - 10) / 2

                var inActive = dataSource.readOnly || (x >= startX && x <= endX)
                ctx.strokeStyle = inActive ? Theme.accentPurple : Theme.borderHover

                ctx.beginPath()
                ctx.moveTo(x, centerY - peakH)
                ctx.lineTo(x, centerY + peakH)
                ctx.stroke()
            }

            if (!dataSource.readOnly) {
                // Start trim marker (teal)
                ctx.strokeStyle = Theme.accentTeal
                ctx.lineWidth = 2
                ctx.beginPath()
                ctx.moveTo(startX, 0)
                ctx.lineTo(startX, graphH)
                ctx.stroke()

                ctx.fillStyle = Theme.accentTeal
                ctx.beginPath()
                ctx.moveTo(startX, 0)
                ctx.lineTo(startX + 6, 0)
                ctx.lineTo(startX, 6)
                ctx.closePath()
                ctx.fill()

                // End trim marker (red)
                ctx.strokeStyle = Theme.destructiveRed
                ctx.lineWidth = 2
                ctx.beginPath()
                ctx.moveTo(endX, 0)
                ctx.lineTo(endX, graphH)
                ctx.stroke()

                ctx.fillStyle = Theme.destructiveRed
                ctx.beginPath()
                ctx.moveTo(endX, 0)
                ctx.lineTo(endX - 6, 0)
                ctx.lineTo(endX, 6)
                ctx.closePath()
                ctx.fill()
            }

            // Playback cursor (white)
            if (dataSource.playPositionMs >= 0) {
                var cursorX = (dataSource.playPositionMs / duration) * w
                ctx.strokeStyle = Theme.textPrimary
                ctx.lineWidth = 2
                ctx.beginPath()
                ctx.moveTo(cursorX, 0)
                ctx.lineTo(cursorX, graphH)
                ctx.stroke()
            }

            // Timeline labels
            ctx.fillStyle = Theme.textDim
            ctx.font = "9px sans-serif"
            ctx.textBaseline = "middle"

            if (dataSource.readOnly) {
                ctx.textAlign = "left"
                ctx.fillText("0.0s", 4, graphH + labelH / 2)
                ctx.textAlign = "right"
                ctx.fillText((duration / 1000).toFixed(1) + "s", w - 4, graphH + labelH / 2)
            } else {
                var startStr = (dataSource.startMs / 1000).toFixed(1) + "s"
                ctx.textAlign = "left"
                ctx.fillText(startStr, 4, graphH + labelH / 2)

                ctx.textAlign = "right"
                ctx.fillText((duration / 1000).toFixed(1) + "s", w - 4, graphH + labelH / 2)

                var rangeSec = (currentEnd - dataSource.startMs) / 1000
                ctx.textAlign = "center"
                ctx.fillText("Crop: " + rangeSec.toFixed(1) + "s", w / 2, graphH + labelH / 2)
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton

        property int dragTarget: 0

        onPressed: function(mouse) {
            if (dataSource.readOnly || !dataSource.valid) return
            var w = width
            var duration = dataSource.durationMs > 0 ? dataSource.durationMs : 1
            var currentEnd = (dataSource.endMs === -1 || dataSource.endMs === 0) ? duration : dataSource.endMs
            var sx = (dataSource.startMs / duration) * w
            var ex = (currentEnd / duration) * w
            var mx = mouse.x

            var distStart = Math.abs(mx - sx)
            var distEnd = Math.abs(mx - ex)

            if (distStart < 15 && distStart <= distEnd)
                dragTarget = 1
            else if (distEnd < 15)
                dragTarget = 2
            else
                dragTarget = 0
        }

        onPositionChanged: function(mouse) {
            if (dataSource.readOnly || !dataSource.valid) return
            var w = width
            if (w <= 0) return
            var duration = dataSource.durationMs > 0 ? dataSource.durationMs : 1
            var pct = Math.max(0, Math.min(1, mouse.x / w))
            var newTimeMs = Math.round(pct * duration)

            if (dragTarget === 1) {
                var currentEnd = (dataSource.endMs === -1 || dataSource.endMs === 0) ? duration : dataSource.endMs
                var clamped = Math.max(0, Math.min(newTimeMs, currentEnd - 50))
                dataSource.setClipRange(clamped, dataSource.endMs)
                root.trimRangeChanged(clamped, dataSource.endMs)
            } else if (dragTarget === 2) {
                var clamped = Math.max(dataSource.startMs + 50, Math.min(newTimeMs, duration))
                dataSource.setClipRange(dataSource.startMs, clamped)
                root.trimRangeChanged(dataSource.startMs, clamped)
            } else {
                var currentEnd2 = (dataSource.endMs === -1 || dataSource.endMs === 0) ? duration : dataSource.endMs
                var sx2 = (dataSource.startMs / duration) * w
                var ex2 = (currentEnd2 / duration) * w
                cursorShape = (Math.abs(mouse.x - sx2) < 10 || Math.abs(mouse.x - ex2) < 10)
                    ? Qt.SplitHCursor : Qt.ArrowCursor
            }
        }

        onReleased: function(mouse) {
            if (dragTarget !== 0)
                root.trimRangeCommit(dataSource.startMs, dataSource.endMs)
            dragTarget = 0
            cursorShape = Qt.ArrowCursor
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 6
        visible: root.showPlaceholder

        // Warning state (if missing)
        Image {
            visible: root.filePath !== "" && !root.fileExists
            source: "image://icons/triangle-alert?color=" + encodeURIComponent(Theme.accentRed)
            sourceSize: Qt.size(16, 16)
            Layout.alignment: Qt.AlignHCenter
        }

        // Empty state (if empty, showing music icon)
        Image {
            visible: root.filePath === "" && root.emptyText !== "Replay buffer empty"
            source: "image://icons/music?color=%23444444"
            sourceSize: Qt.size(16, 16)
            Layout.alignment: Qt.AlignHCenter
        }

        // Replay buffer empty state (headphones icon)
        Image {
            visible: root.filePath === "" && root.emptyText === "Replay buffer empty"
            source: "image://icons/headphones?color=%23444444"
            sourceSize: Qt.size(16, 16)
            Layout.alignment: Qt.AlignHCenter
        }

        // Loading text
        Text {
            visible: root.filePath !== "" && root.fileExists && !dataSource.valid
            text: "Loading waveform..."
            color: Theme.textDim
            font.pixelSize: 10
            Layout.alignment: Qt.AlignHCenter
        }

        // Empty text
        Text {
            visible: root.filePath === "" && (!dataSource.valid || !dataSource.peaks || dataSource.peaks.length === 0)
            text: root.emptyText
            color: Theme.textDim
            font.pixelSize: 10
            Layout.alignment: Qt.AlignHCenter
        }

        // Missing text
        Text {
            visible: root.filePath !== "" && !root.fileExists
            text: "Audio file missing"
            color: Theme.accentRed
            font.pixelSize: 10
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
