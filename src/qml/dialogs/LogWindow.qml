import QtQuick 2.15
import QtQuick.Window 2.15
import Saiko 1.0

// Standalone window wrapping the LogViewer component.
Window {
    id: root
    width: 720
    height: 400
    minimumWidth: 500
    minimumHeight: 250
    color: Theme.appBackground
    title: "Log Viewer"

    signal opened()
    signal closed()

    onVisibleChanged: {
        if (visible)
            opened();
    }

    onClosing: {
        closed();
    }

    LogViewer {
        id: logViewer
        anchors.fill: parent
    }
}
