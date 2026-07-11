import QtQuick 2.15
import QtQuick.Controls 2.15
import Saiko 1.0

Popup {
    id: root

    property string text: ""
    property bool hovered: false
    property string direction: "right" // Supported: "top", "bottom", "left", "right"

    // --- GLOBAL OVERLAY MAGIC ---
    // Automatically lifts the tooltip onto the window's top rendering layer.
    // Its x and y coordinates are still calculated relative to its button parent!
    visible: root.hovered && text.length > 0
    closePolicy: Popup.NoAutoClose
    padding: 0
    enabled: false

    width: textItem.implicitWidth + 16
    height: 26

    background: Rectangle {
        color: "#232323"
        border.color: Theme.borderDefault
        border.width: 1
        radius: 6
    }

    contentItem: Text {
        id: textItem
        text: root.text
        color: Theme.textPrimary
        font.pixelSize: 11
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    // Dynamic X Coordinate Math (Relative to the host button)
    x: {
        if (!parent) return 0;
        if (root.direction === "left") {
            return root.hovered ? -width - 8 : -width - 24;
        } else if (root.direction === "right") {
            return root.hovered ? parent.width + 8 : parent.width + 24;
        } else {
            // Horizontal Center alignment for "top" and "bottom" directions
            return (parent.width - width) / 2;
        }
    }

    // Dynamic Y Coordinate Math (Relative to the host button)
    y: {
        if (!parent) return 0;
        if (root.direction === "top") {
            return root.hovered ? -height - 8 : -height - 24;
        } else if (root.direction === "bottom") {
            return root.hovered ? parent.height + 8 : parent.height + 24;
        } else {
            // Vertical Center alignment for "left" and "right" directions
            return (parent.height - height) / 2;
        }
    }

    // Fluid slide animations across structural panel splits
    Behavior on x { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
    Behavior on y { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }

    // Clean fade transitions natively managed by the Popup system
    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 150 }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 150 }
    }
}