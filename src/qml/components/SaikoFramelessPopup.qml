import QtQuick 2.15
import QtQuick.Controls 2.15
import Saiko 1.0

Popup {
    id: root
    parent: Overlay.overlay
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    modal: true
    dim: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    padding: 0

    property real contentScale: 1.0

    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: Theme.animDuration; easing.type: Easing.OutQuad }
            NumberAnimation { target: root; property: "contentScale"; from: 0.95; to: 1.0; duration: Theme.animDuration; easing.type: Easing.OutQuad }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: Theme.animDuration; easing.type: Easing.OutQuad }
            NumberAnimation { target: root; property: "contentScale"; from: 1.0; to: 0.95; duration: Theme.animDuration; easing.type: Easing.OutQuad }
        }
    }

    background: Rectangle {
        color: Theme.appBackground
        border.color: Theme.borderDefault
        border.width: 1
        radius: Theme.cardRadius || 8
        scale: root.contentScale
        transformOrigin: Item.Center

        Rectangle {
            anchors.fill: parent
            anchors.margins: -1
            color: "transparent"
            border.color: Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.15)
            border.width: 1
            radius: (Theme.cardRadius || 8) + 1
            z: -1
        }
    }

    contentItem: Item {
        scale: root.contentScale
        transformOrigin: Item.Center

        SaikoIconButton {
            id: closeButton
            textIcon: Qt.platform.os === "windows" ? "\uE8BB" : "\u2715"
            textIconFontFamily: Qt.platform.os === "windows" ? "Segoe MDL2 Assets" : "Segoe UI"
            textIconSize: Qt.platform.os === "windows" ? 10 : 12
            tooltipText: "Close"
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 20
            anchors.rightMargin: 20
            onClicked: root.close()
        }
    }

    function show() { open(); }
    function hide() { close(); }
}
