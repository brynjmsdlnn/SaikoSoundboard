import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Templates 2.15 as T

T.CheckBox {
    id: cb
    font.pixelSize: 12
    hoverEnabled: true
    spacing: 12

    property int indicatorSize: font.pixelSize >= 12 ? 18 : 14
    property int radius: font.pixelSize >= 12 ? 4 : 3

    implicitWidth: contentItem.implicitWidth
    implicitHeight: Math.max(indicator.implicitHeight, contentItem.implicitHeight)

    indicator: Rectangle {
        implicitWidth: cb.indicatorSize
        implicitHeight: cb.indicatorSize
        x: cb.leftPadding
        y: parent.height / 2 - height / 2
        radius: cb.radius
        color: cb.checked ? "#BB86FC" : (cb.hovered ? "#1a1a1a" : "#121212")
        border.color: cb.checked ? "#BB86FC" : (cb.hovered ? "#333" : "#1c1c1c")
        border.width: 1

        Behavior on color { ColorAnimation { duration: 150 } }
        Behavior on border.color { ColorAnimation { duration: 150 } }

        Text {
            text: "✓"
            color: "black"
            font.pixelSize: cb.font.pixelSize >= 12 ? 11 : 9
            font.weight: Font.Bold
            anchors.centerIn: parent
            visible: cb.checked
        }
    }

    contentItem: Text {
        text: cb.text
        font: cb.font
        color: cb.checked ? "white" : (cb.hovered ? "#bbb" : "#888")
        leftPadding: cb.indicator.width + cb.spacing
        verticalAlignment: Text.AlignVCenter
        Behavior on color { ColorAnimation { duration: 150 } }
    }
}
