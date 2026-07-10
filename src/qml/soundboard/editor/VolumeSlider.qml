import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

ColumnLayout {
    id: root

    property real volume: 1.0
    property bool isLocked: false
    property int slotIndex: -1
    property var slotModel: null

    Layout.fillHeight: true
    Layout.minimumWidth: 60
    spacing: 8

    SaikoSectionLabel {
        text: "VOLUME"
        Layout.alignment: Qt.AlignHCenter
    }

    Slider {
        id: volSlider
        orientation: Qt.Vertical
        Layout.fillHeight: true
        enabled: !root.isLocked
        Layout.alignment: Qt.AlignHCenter
        from: 0
        to: 100
        value: root.volume * 100
        live: true

        background: Rectangle {
            x: volSlider.leftPadding + volSlider.availableWidth / 2 - width / 2
            y: volSlider.topPadding
            width: 4
            height: volSlider.availableHeight
            radius: 2
            color: Theme.borderDefault

            Rectangle {
                y: volSlider.visualPosition * parent.height
                width: parent.width
                height: parent.height - y
                color: Theme.accentPurple
                radius: 2
            }
        }

        handle: Rectangle {
            x: volSlider.leftPadding + volSlider.availableWidth / 2 - width / 2
            y: volSlider.topPadding + volSlider.visualPosition * volSlider.availableHeight - height / 2
            width: 14
            height: 14
            radius: 7
            color: volSlider.pressed ? Theme.accentPurple : Theme.textPrimary
            border.color: Theme.accentPurple
            border.width: 1
        }

        onMoved: {
            if (root.slotIndex >= 0)
                root.slotModel.setVolume(root.slotIndex, value / 100)
        }
    }

    Text {
        text: (root.volume * 100).toFixed(0) + "%"
        color: Theme.textPrimary
        font.pixelSize: 12
        font.bold: true
        Layout.alignment: Qt.AlignHCenter
    }
}
