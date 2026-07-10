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

    // ── Private state for icon cross-fade animation ────────────────
    property string __previousIconSource: ""

    onIconSourceChanged: {
        if (__previousIconSource !== "" && __previousIconSource !== iconSource) {
            // Set initial states instantly, then cross-fade after images load
            fadeOutIcon.source = __previousIconSource
            fadeOutIcon.opacity = 1.0
            activeIcon.opacity = 0.0
            crossFadeTimer.restart()
        }
        __previousIconSource = iconSource
    }

    // ── Animated Icon ──────────────────────────────────────────────
    Item {
        anchors.centerIn: parent
        width: 14
        height: 14

        scale: area.pressed ? 0.75 : 1.0
        Behavior on scale {
            NumberAnimation {
                duration: 150
                easing.type: Easing.OutBack
            }
        }

        // Fading-out icon layer (old icon during cross-fade)
        Image {
            id: fadeOutIcon
            anchors.fill: parent
            sourceSize: Qt.size(14, 14)
            opacity: 0
            visible: opacity > 0
        }

        // Active icon layer (new icon during cross-fade, current icon otherwise)
        Image {
            id: activeIcon
            anchors.fill: parent
            sourceSize: Qt.size(14, 14)
            source: (btn.hoveredIconSource && area.containsMouse)
                ? btn.hoveredIconSource
                : btn.iconSource
        }

        // Explicit property animations for smooth cross-fade
        PropertyAnimation {
            id: fadeOutAnim
            target: fadeOutIcon
            property: "opacity"
            to: 0
            duration: 200
            easing.type: Easing.InOutQuad
        }

        PropertyAnimation {
            id: fadeInAnim
            target: activeIcon
            property: "opacity"
            to: 1
            duration: 200
            easing.type: Easing.InOutQuad
        }

        // Small delay to ensure fadeOutIcon source is loaded before animating
        Timer {
            id: crossFadeTimer
            interval: 16
            repeat: false
            onTriggered: {
                fadeOutAnim.start()
                fadeInAnim.start()
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
