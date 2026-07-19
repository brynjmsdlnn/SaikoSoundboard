import QtQuick 2.15
import QtQuick.Window 2.15
import Saiko 1.0

Window {
    id: overlayWindow
    title: "Saiko Notification Overlay"
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus | Qt.WindowTransparentForInput
    color: "transparent"

    // Dynamically fit to the notification list with 10px padding on each side
    width: notificationList.width + 20
    height: notificationList.height + 20

    // Helper: compute x/y based on position setting
    function computePosition() {
        var pos = Backend.notifications.position;
        var margin = 20;
        switch (pos) {
        case "TopLeft":
            overlayWindow.x = margin;
            overlayWindow.y = margin;
            break;
        case "TopRight":
            overlayWindow.x = Screen.desktopAvailableWidth - overlayWindow.width - margin;
            overlayWindow.y = margin;
            break;
        case "BottomLeft":
            overlayWindow.x = margin;
            overlayWindow.y = Screen.desktopAvailableHeight - overlayWindow.height - margin;
            break;
        case "BottomRight":
        default:
            overlayWindow.x = Screen.desktopAvailableWidth - overlayWindow.width - margin;
            overlayWindow.y = Screen.desktopAvailableHeight - overlayWindow.height - margin;
            break;
        }
    }

    Component.onCompleted: computePosition()

    onWidthChanged: computePosition()
    onHeightChanged: computePosition()

    visible: notificationList.count > 0

    onVisibleChanged: {
        if (visible) {
            computePosition();
        }
    }

    Connections {
        target: Backend.notifications
        function onPositionChanged() {
            computePosition();
        }
    }

    NotificationList {
        id: notificationList
        isOverlayMode: true
        x: 10
        y: 10
    }
}
