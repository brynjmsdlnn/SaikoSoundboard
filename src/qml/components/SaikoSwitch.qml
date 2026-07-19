import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Templates 2.15 as T
import Saiko 1.0

T.Switch {
    id: sw
    font.pixelSize: 12
    hoverEnabled: true
    spacing: 12

    property int trackWidth: 40
    property int trackHeight: 22
    property int handleSize: 14
    property int handleMargin: (trackHeight - handleSize) / 2

    implicitWidth: text !== "" ? contentItem.implicitWidth + trackWidth + spacing : indicator.implicitWidth
    implicitHeight: Math.max(trackHeight, contentItem.implicitHeight)

    indicator: Rectangle {
        implicitWidth: sw.trackWidth
        implicitHeight: sw.trackHeight
        x: sw.leftPadding
        y: parent.height / 2 - height / 2
        radius: height / 2
        color: sw.checked ? Theme.accentPurple : Theme.borderDefault
        border.color: Theme.borderDefault
        border.width: 1

        Behavior on color { ColorAnimation { duration: 150 } }

        Rectangle {
            width: sw.handleSize
            height: sw.handleSize
            radius: width / 2
            color: "#FFFFFF"
            y: sw.handleMargin
            x: sw.checked ? parent.width - width - sw.handleMargin : sw.handleMargin

            Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
        }
    }

    contentItem: Text {
        text: sw.text
        font: sw.font
        color: sw.checked ? Theme.textPrimary : (sw.hovered ? Theme.textSecondary : Theme.textDim)
        leftPadding: sw.indicator.width + sw.spacing
        verticalAlignment: Text.AlignVCenter
        Behavior on color { ColorAnimation { duration: 150 } }
    }
}
