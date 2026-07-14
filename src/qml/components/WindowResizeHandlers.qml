import QtQuick 2.15
import QtQuick.Window 2.15
import Saiko 1.0

// ─────────────────────────────────────────────────────────────────────────────
// Invisible resize handles for a frameless ApplicationWindow/Window.
// Provides 4 edge MouseAreas (top/bottom/left/right), 4 corner MouseAreas,
// and a thin outer border that is hidden when the window is maximized.
//
// Must be a direct child of the Window/ApplicationWindow and stacked below
// the content (use lower z-values). All MouseAreas are disabled when the
// window is maximized.
// ─────────────────────────────────────────────────────────────────────────────
Item {
    id: resizeHandlers
    z: 2000
    enabled: Qt.platform.os !== "windows"

    readonly property var targetWindow: Window.window

    // Thickness of the edge resize zones. Corners use double this value.
    property int borderMargin: 4

    // Helper properties to check maximized state safely
    readonly property bool isMaximized: targetWindow && targetWindow.visibility === Window.Maximized

    // ── Outer Window Border (visual distinction, hidden when maximized) ─────
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: Theme.borderDefault
        border.width: 1
        z: 9998
        visible: !resizeHandlers.isMaximized
    }

    // ── Edge resize zones ────────────────────────────────────────────────────
    // Top
    MouseArea {
        height: borderMargin
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: borderMargin
        anchors.rightMargin: borderMargin
        cursorShape: Qt.SizeVerCursor
        acceptedButtons: Qt.LeftButton
        enabled: !resizeHandlers.isMaximized
        onPressed: {
            if (resizeHandlers.targetWindow)
                resizeHandlers.targetWindow.startSystemResize(Qt.TopEdge)
        }
        z: 9999
    }

    // Bottom
    MouseArea {
        height: borderMargin
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: borderMargin
        anchors.rightMargin: borderMargin
        cursorShape: Qt.SizeVerCursor
        acceptedButtons: Qt.LeftButton
        enabled: !resizeHandlers.isMaximized
        onPressed: {
            if (resizeHandlers.targetWindow)
                resizeHandlers.targetWindow.startSystemResize(Qt.BottomEdge)
        }
        z: 9999
    }

    // Left
    MouseArea {
        width: borderMargin
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: borderMargin
        anchors.bottomMargin: borderMargin
        cursorShape: Qt.SizeHorCursor
        acceptedButtons: Qt.LeftButton
        enabled: !resizeHandlers.isMaximized
        onPressed: {
            if (resizeHandlers.targetWindow)
                resizeHandlers.targetWindow.startSystemResize(Qt.LeftEdge)
        }
        z: 9999
    }

    // Right
    MouseArea {
        width: borderMargin
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: borderMargin
        anchors.bottomMargin: borderMargin
        cursorShape: Qt.SizeHorCursor
        acceptedButtons: Qt.LeftButton
        enabled: !resizeHandlers.isMaximized
        onPressed: {
            if (resizeHandlers.targetWindow)
                resizeHandlers.targetWindow.startSystemResize(Qt.RightEdge)
        }
        z: 9999
    }

    // ── Corner resize zones (higher z to take priority over edges) ──────────
    // Top-Left
    MouseArea {
        width: borderMargin * 2
        height: borderMargin * 2
        anchors.top: parent.top
        anchors.left: parent.left
        cursorShape: Qt.SizeFDiagCursor
        acceptedButtons: Qt.LeftButton
        enabled: !resizeHandlers.isMaximized
        onPressed: {
            if (resizeHandlers.targetWindow)
                resizeHandlers.targetWindow.startSystemResize(Qt.TopEdge | Qt.LeftEdge)
        }
        z: 10000
    }

    // Top-Right
    MouseArea {
        width: borderMargin * 2
        height: borderMargin * 2
        anchors.top: parent.top
        anchors.right: parent.right
        cursorShape: Qt.SizeBDiagCursor
        acceptedButtons: Qt.LeftButton
        enabled: !resizeHandlers.isMaximized
        onPressed: {
            if (resizeHandlers.targetWindow)
                resizeHandlers.targetWindow.startSystemResize(Qt.TopEdge | Qt.RightEdge)
        }
        z: 10000
    }

    // Bottom-Left
    MouseArea {
        width: borderMargin * 2
        height: borderMargin * 2
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        cursorShape: Qt.SizeBDiagCursor
        acceptedButtons: Qt.LeftButton
        enabled: !resizeHandlers.isMaximized
        onPressed: {
            if (resizeHandlers.targetWindow)
                resizeHandlers.targetWindow.startSystemResize(Qt.BottomEdge | Qt.LeftEdge)
        }
        z: 10000
    }

    // Bottom-Right
    MouseArea {
        width: borderMargin * 2
        height: borderMargin * 2
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        cursorShape: Qt.SizeFDiagCursor
        acceptedButtons: Qt.LeftButton
        enabled: !resizeHandlers.isMaximized
        onPressed: {
            if (resizeHandlers.targetWindow)
                resizeHandlers.targetWindow.startSystemResize(Qt.BottomEdge | Qt.RightEdge)
        }
        z: 10000
    }
}
