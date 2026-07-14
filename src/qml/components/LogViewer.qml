import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

// Log viewer panel — shows real-time log entries from LogModel.
// Designed to be embedded as a bottom dock in the main window.
Rectangle {
    id: root

    color: Theme.cardBackground
    border.color: Theme.borderDefault
    border.width: 1
    radius: Theme.cardRadius
    clip: true

    // Color mapping matching ConsoleSink's ANSI colors
    readonly property var levelColors: [
        "#888888",  // Trace  — gray
        "#22dddd",  // Debug  — cyan
        "#44cc44",  // Info   — green
        "#ddaa33",  // Warn   — yellow
        "#dd4444",  // Error  — red
        "#ff5555"   // Critical — bright red
    ]

    readonly property var levelNames: [
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRIT"
    ]

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Toolbar ────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: Theme.recessedBackground
            border.color: Theme.borderDefault
            border.width: 1
            radius: Theme.cardRadius

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 6
                anchors.topMargin: 2
                anchors.bottomMargin: 2
                spacing: 6

                Text {
                    text: "Log"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeNormal
                    font.weight: Font.Bold
                }

                // Open logs folder button
                SaikoIconButton {
                    implicitWidth: 26
                    implicitHeight: 24
                    isActive: true
                    tooltipText: "Open logs folder"
                    tooltipDirection: "right"
                    iconSource: "image://icons/folder?color=%23b0b0b0"
                    onClicked: {
                        Qt.openUrlExternally("file:///" + encodeURI(Backend.logDirectory()))
                    }
                }

                Item { Layout.fillWidth: true }

                // Filter combo
                SaikoComboBox {
                    id: levelFilter
                    model: ["All", "DEBUG+", "INFO+", "WARN+", "ERROR+", "CRIT+"]
                    textRole: ""
                    valueRole: ""
                    currentIndex: 0
                    Layout.preferredWidth: 96
                    implicitHeight: 26
                    radius: 5
                    font.pixelSize: 11
                    isActive: true

                    onActivated: function(index) {
                        LogModel.filterLevel = index
                    }

                    SaikoTooltip {
                        text: "Minimum log level"
                        hovered: levelFilter.hovered
                        direction: "left"
                    }
                }

                // Clear button
                SaikoIconButton {
                    implicitWidth: 26
                    implicitHeight: 24
                    isActive: true
                    tooltipText: "Clear logs"
                    tooltipDirection: "left"
                    iconSource: "image://icons/trash?color=%23b0b0b0"
                    onClicked: LogModel.clear()
                }


            }
        }

        // ── Log list ────────────────────────────────────────────
        ListView {
            id: logList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            model: LogModel
            spacing: 0

            delegate: Rectangle {
                id: delegateRoot
                width: logList.width
                height: 20
                color: index % 2 === 0 ? "transparent" : Qt.rgba(1, 1, 1, 0.02)

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 8

                    // Colored level indicator
                    Rectangle {
                        width: 44
                        height: 14
                        radius: 2
                        color: levelColors[model.level] || "#888888"

                        Text {
                            anchors.centerIn: parent
                            text: model.levelName
                            color: "#000000"
                            font.pixelSize: 9
                            font.weight: Font.Bold
                        }
                    }

                    // Timestamp
                    Text {
                        text: model.timestampDisplay
                        color: Theme.textDim
                        font.pixelSize: 10
                        font.family: "Consolas, Courier New, monospace"
                        Layout.minimumWidth: 60
                        Layout.preferredWidth: 75
                    }

                    // Category
                    Text {
                        text: model.category
                        color: Theme.textSecondary
                        font.pixelSize: 10
                        Layout.minimumWidth: 50
                        Layout.preferredWidth: 80
                        elide: Text.ElideRight
                    }

                    // Source location
                    Text {
                        text: model.sourceLocation
                        color: Theme.textDim
                        font.pixelSize: 9
                        font.family: "Consolas, Courier New, monospace"
                        Layout.minimumWidth: 60
                        Layout.preferredWidth: 110
                        elide: Text.ElideRight
                    }

                    // Message
                    Text {
                        text: model.message
                        color: {
                            if (model.level >= 4) return Theme.errorDefault      // Error+
                            if (model.level >= 3) return Theme.warning            // Warning
                            return levelColors[model.level] || Theme.textPrimary
                        }
                        font.pixelSize: 10
                        font.family: "Consolas, Courier New, monospace"
                        Layout.fillWidth: true
                        Layout.minimumWidth: 80
                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: delegateRoot.color = Qt.rgba(1, 1, 1, 0.06)
                    onExited: delegateRoot.color = index % 2 === 0 ? "transparent" : Qt.rgba(1, 1, 1, 0.02)
                }
            }

            ScrollBar.vertical: ScrollBar {
                active: true
                policy: ScrollBar.AsNeeded
                background: Rectangle { color: Theme.recessedBackground; radius: 2 }
                contentItem: Rectangle {
                    color: Theme.borderHover
                    radius: 2
                    implicitWidth: 6
                }
            }

            // Empty state
            Text {
                anchors.centerIn: parent
                text: "No log entries yet"
                color: Theme.textDim
                font.pixelSize: Theme.fontSizeNormal
                visible: logList.count === 0
            }
        }
    }
}
