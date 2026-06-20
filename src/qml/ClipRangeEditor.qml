import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Rectangle {
    id: root

    property int startMs: 0
    property int endMs: 0
    property real durationSec: 1

    signal clipRangeChanged(int start, int end)

    Layout.fillWidth: true
    Layout.preferredHeight: clipColumn.implicitHeight + 16
    color: Theme.cardBackground
    radius: Theme.cardRadius
    border.color: Theme.borderDefault
    border.width: 1

    component ClipSpinBox: SpinBox {
        id: ctrl
        Layout.fillWidth: true
        implicitHeight: 24
        from: 0
        to: Math.max(1, root.durationSec * 1000)
        stepSize: 100
        editable: true
        font.pixelSize: 9

        contentItem: TextInput {
            text: ctrl.textFromValue(ctrl.value, ctrl.locale)
            color: Theme.textPrimary
            font: ctrl.font
            readOnly: !ctrl.editable
            validator: ctrl.validator
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            verticalAlignment: TextInput.AlignVCenter
            horizontalAlignment: TextInput.AlignHCenter
            selectByMouse: true
        }

        background: Rectangle {
            implicitHeight: 24
            color: Theme.recessedBackground
            radius: 5
            border.width: 1
            border.color: ctrl.activeFocus ? Theme.accentPurple : (ctrl.hovered ? Theme.borderHover : Theme.borderDefault)
            Behavior on border.color { ColorAnimation { duration: 100 } }
        }

        textFromValue: function(val) { return (val / 1000).toFixed(1) + "s" }
        valueFromText: function(text) {
            var v = parseFloat(text)
            return isNaN(v) ? ctrl.value : Math.round(v * 1000)
        }
    }

    ColumnLayout {
        id: clipColumn
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        Text {
            text: "SELECTED CLIP"
            color: Theme.textDim
            font.pixelSize: 8
            font.letterSpacing: 1.2
            font.weight: Font.Bold
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text { text: "Start"; color: Theme.textDim; font.pixelSize: 9; Layout.preferredWidth: 30 }

            ClipSpinBox {
                id: startSpin
                value: root.startMs

                Binding {
                    target: startSpin
                    property: "value"
                    value: root.startMs
                }

                onValueChanged: {
                    if (activeFocus) {
                        var endVal = endSpin.value
                        if (value > endVal - 50) value = Math.max(from, endVal - 50)
                        root.clipRangeChanged(value, endSpin.value)
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text { text: "End"; color: Theme.textDim; font.pixelSize: 9; Layout.preferredWidth: 30 }

            ClipSpinBox {
                id: endSpin
                value: root.endMs === -1 ? Math.max(1, root.durationSec * 1000) : root.endMs

                Binding {
                    target: endSpin
                    property: "value"
                    value: root.endMs === -1 ? Math.max(1, root.durationSec * 1000) : root.endMs
                }

                onValueChanged: {
                    if (activeFocus) {
                        var startVal = startSpin.value
                        if (value < startVal + 50) value = startVal + 50
                        root.clipRangeChanged(startSpin.value, value)
                    }
                }
            }
        }
    }
}
