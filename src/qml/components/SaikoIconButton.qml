import QtQuick 2.15
import QtQuick.Controls 2.15
import Saiko 1.0

Rectangle {
    id: btn

    // ── Public API ──────────────────────────────────────────────────────────
    property string iconSource: ""
    property string hoveredIconSource: ""
    property string tooltipText: ""
    property string tooltipDirection: "top"
    property bool isActive: true
    property color hoverColor: "#1a1a1a"
    property color hoverBorderColor: Theme.borderHover

    readonly property alias containsMouse: area.containsMouse

    signal clicked()

    // ── Appearance ──────────────────────────────────────────────────────────
    implicitWidth: 32
    implicitHeight: 28
    radius: 6

    color: area.containsMouse ? btn.hoverColor : "transparent"
    border.color: area.containsMouse ? btn.hoverBorderColor : "transparent"
    border.width: 1

    opacity: btn.isActive ? 1.0 : 0.4

    Behavior on color {
        ColorAnimation { duration: 150 }
    }
    Behavior on border.color {
        ColorAnimation { duration: 150 }
    }
    Behavior on opacity {
        NumberAnimation { duration: 150 }
    }

    // ── Icon ────────────────────────────────────────────────────────────────
    Image {
        anchors.centerIn: parent
        sourceSize: Qt.size(14, 14)
        source: (btn.hoveredIconSource && area.containsMouse)
            ? btn.hoveredIconSource
            : btn.iconSource

        scale: area.pressed ? 0.75 : 1.0
        Behavior on scale {
            NumberAnimation {
                duration: 150
                easing.type: Easing.OutBack
            }
        }
    }

    // ── Tooltip ─────────────────────────────────────────────────────────────
    SaikoTooltip {
        text: btn.tooltipText
        hovered: area.containsMouse
        direction: btn.tooltipDirection
    }

    // ── Interaction ─────────────────────────────────────────────────────────
    MouseArea {
        id: area
        anchors.fill: parent
        cursorShape: btn.isActive ? Qt.PointingHandCursor : Qt.ArrowCursor
        hoverEnabled: true
        enabled: btn.isActive
        onClicked: btn.clicked()
    }
}
