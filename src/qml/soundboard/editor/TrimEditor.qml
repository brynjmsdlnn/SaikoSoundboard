import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

ColumnLayout {
    id: root

    property int startTimeMs: 0
    property int endTimeMs: 0
    property var waveformData: null
    property bool isLocked: false
    property string filePath: ""
    property bool fileExists: true
    property int slotIndex: -1
    property var slotModel: null
    property var layerPositionsMs: []

    property bool showSavedStatus: false

    Timer {
        id: saveTimer
        interval: 1500
        onTriggered: root.showSavedStatus = false
    }

    signal trimRangeChanged(int startMs, int endMs)
    signal trimRangeCommit(int startMs, int endMs)

    Layout.fillWidth: true
    spacing: 8

    RowLayout {
        Layout.fillWidth: true
        spacing: 8

        SaikoSectionLabel {
            text: "TRIM & TIMING"
        }

        Item { Layout.fillWidth: true }

        Text {
            id: resetBtn
            text: "Reset"
            color: resetMouse.containsMouse ? Theme.accentPurple : Theme.textDim
            font.pixelSize: 10
            font.bold: true
            visible: !root.isLocked && !wf.isDragging && !root.showSavedStatus && (root.startTimeMs > 0 || root.endTimeMs !== -1)

            Behavior on color { ColorAnimation { duration: 100 } }

            MouseArea {
                id: resetMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (root.slotIndex >= 0) {
                        root.slotModel.setClipRange(root.slotIndex, 0, -1, true)
                    }
                }
            }
        }

        Text {
            text: wf.isDragging ? "EDITING" : "SAVED"
            color: wf.isDragging ? Theme.accentPurple : Theme.accentGreen
            font.pixelSize: 10
            font.bold: true
            visible: wf.isDragging || root.showSavedStatus

            SequentialAnimation on opacity {
                running: wf.isDragging
                loops: Animation.Infinite
                NumberAnimation { from: 1.0; to: 0.4; duration: 600; easing.type: Easing.InOutQuad }
                NumberAnimation { from: 0.4; to: 1.0; duration: 600; easing.type: Easing.InOutQuad }
            }

            onVisibleChanged: {
                if (!visible) opacity = 1.0
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        height: 72
        color: Theme.recessedBackground
        radius: 6
        border.color: {
            if (wf.isDragging) return Theme.accentPurple
            if (root.showSavedStatus) return Theme.accentGreen
            return Theme.borderDefault
        }
        clip: true

        Behavior on border.color { ColorAnimation { duration: 150 } }

        WaveformView {
            id: wf
            anchors { fill: parent; margins: 6 }
            startMs: root.startTimeMs
            endMs: root.endTimeMs
            waveformData: root.waveformData
            readOnly: root.isLocked
            filePath: root.filePath
            fileExists: root.fileExists
            emptyText: "No file assigned"
            layerPositionsMs: root.layerPositionsMs

            onIsDraggingChanged: {
                if (!isDragging) {
                    root.showSavedStatus = true
                    saveTimer.restart()
                } else {
                    saveTimer.stop()
                    root.showSavedStatus = false
                }
            }

            onTrimRangeChanged: (s, e) => {
                if (!root.isLocked && root.slotIndex >= 0)
                    root.slotModel.setClipRange(root.slotIndex, s, e, false)
            }
            onTrimRangeCommit: (s, e) => {
                if (!root.isLocked && root.slotIndex >= 0)
                    root.slotModel.setClipRange(root.slotIndex, s, e, true)
            }
        }
    }

    function setPlayPositionMs(pos) {
        wf.playPositionMs = pos
    }
}
