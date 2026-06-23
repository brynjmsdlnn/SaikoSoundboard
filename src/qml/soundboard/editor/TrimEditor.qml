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
    property int slotIndex: -1
    property var slotModel: null

    signal trimRangeChanged(int startMs, int endMs)
    signal trimRangeCommit(int startMs, int endMs)

    Layout.fillWidth: true
    spacing: 8

    SectionLabel {
        text: "TRIM & TIMING"
    }

    Rectangle {
        Layout.fillWidth: true
        height: 72
        color: Theme.recessedBackground
        radius: 6
        border.color: Theme.borderDefault
        clip: true

        WaveformView {
            id: wf
            anchors { fill: parent; margins: 6 }
            startMs: root.startTimeMs
            endMs: root.endTimeMs
            waveformData: root.waveformData
            readOnly: root.isLocked

            onTrimRangeChanged: (s, e) => {
                if (!root.isLocked && root.slotIndex >= 0)
                    root.slotModel.setClipRange(root.slotIndex, s, e)
            }
            onTrimRangeCommit: (s, e) => {
                if (!root.isLocked && root.slotIndex >= 0)
                    root.slotModel.setClipRange(root.slotIndex, s, e)
            }
        }
    }

    function setPlayPositionMs(pos) {
        wf.playPositionMs = pos
    }
}
