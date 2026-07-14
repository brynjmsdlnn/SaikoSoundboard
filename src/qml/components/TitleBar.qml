import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Saiko 1.0

// ─────────────────────────────────────────────────────────────────────────────
// Custom frameless title bar with window dragging, double-click maximize,
// and standard window control buttons (minimize, maximize/restore, close).
// Must be placed as a direct child (or descendant) of a Window/ApplicationWindow.
// Uses Window.window to communicate with the parent window.
// ─────────────────────────────────────────────────────────────────────────────
Rectangle {
    id: titleBar

    height: Backend.titleBarHeight
    color: Theme.recessedBackground
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    z: 1000

    readonly property var targetWindow: Window.window

    // ── Window dragging + Aero snap + drag-to-restore ────────────────────────
    DragHandler {
        id: dragHandler
        enabled: Qt.platform.os !== "windows"
        grabPermissions: PointerHandler.CanTakeOverFromAnything
        onActiveChanged: {
            if (active) {
                var win = titleBar.targetWindow;
                if (!win) return;
                if (win.visibility === Window.Maximized) {
                    // Map local cursor point to global screen coordinates
                    var localPoint = centroid.position;
                    var globalPoint = titleBar.mapToItem(null, localPoint.x, localPoint.y);
                    var screenPoint = win.mapToGlobal(globalPoint.x, globalPoint.y);
                    
                    // Calculate relative horizontal ratio of cursor on maximized window
                    var ratio = localPoint.x / win.width;
                    
                    // Use win.normalWidth if declared on the main window, otherwise fallback
                    var restoreWidth = win.normalWidth !== undefined ? win.normalWidth : 1100;
                    
                    // Calculate new coordinates so mouse sits at same proportional location on the title bar
                    var newX = screenPoint.x - (ratio * restoreWidth);
                    var newY = screenPoint.y - localPoint.y;
                    
                    // Ensure the restored window fits on screen
                    newX = Math.max(0, newX);
                    
                    win.showNormal();
                    win.setX(newX);
                    win.setY(newY);
                }
                win.startSystemMove();
            }
        }
    }

    // ── Double-click to toggle maximize ──────────────────────────────────────
    TapHandler {
        enabled: Qt.platform.os !== "windows"
        onDoubleTapped: {
            var win = titleBar.targetWindow;
            if (!win) return;
            if (win.visibility === Window.Maximized) {
                win.showNormal();
            } else {
                win.showMaximized();
            }
        }
    }

    // ── App icon + title ─────────────────────────────────────────────────────
    RowLayout {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        Image {
            source: "image://icons/radio?color=%23" + (Backend.isPlaying ? "bb86fc" : "e35d5d")
            width: 14
            height: 14
        }

        Text {
            text: "Saiko Soundboard"
            color: Theme.textPrimary
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }
    }

    // ── Window control buttons ──────────────────────────────────────────────
    Row {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        layoutDirection: Qt.RightToLeft

        // Close button
        Button {
            id: closeBtn
            width: 45
            height: parent.height
            hoverEnabled: true
            flat: true
            background: Rectangle {
                color: closeBtn.hovered ? Theme.destructiveRed : "transparent"
            }
            contentItem: Text {
                text: "\u2715"
                color: closeBtn.hovered ? "#ffffff" : Theme.textSecondary
                font.pixelSize: 12
                font.family: "Segoe UI"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
                if (titleBar.targetWindow) {
                    titleBar.targetWindow.close();
                }
            }
        }

        // Maximize / Restore button
        Button {
            id: maxBtn
            width: 45
            height: parent.height
            hoverEnabled: true
            flat: true
            background: Rectangle {
                color: maxBtn.hovered ? Qt.rgba(255, 255, 255, 0.08) : "transparent"
            }
            contentItem: Text {
                text: (titleBar.targetWindow && titleBar.targetWindow.visibility === Window.Maximized) ? "\u{1F5D7}" : "\u{1F5D6}"
                color: maxBtn.hovered ? Theme.textPrimary : Theme.textSecondary
                font.pixelSize: 11
                font.family: "Segoe UI"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
                var win = titleBar.targetWindow;
                if (!win) return;
                if (win.visibility === Window.Maximized) {
                    win.showNormal();
                } else {
                    win.showMaximized();
                }
            }
        }

        // Minimize button
        Button {
            id: minBtn
            width: 45
            height: parent.height
            hoverEnabled: true
            flat: true
            background: Rectangle {
                color: minBtn.hovered ? Qt.rgba(255, 255, 255, 0.08) : "transparent"
            }
            contentItem: Text {
                text: "\u{1F5D5}"
                color: minBtn.hovered ? Theme.textPrimary : Theme.textSecondary
                font.pixelSize: 10
                font.family: "Segoe UI"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
                if (titleBar.targetWindow) {
                    titleBar.targetWindow.showMinimized();
                }
            }
        }
    }

    // ── Bottom separator line ────────────────────────────────────────────────
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: Theme.borderDefault
    }
}
