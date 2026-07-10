import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

ColumnLayout {
    id: root

    property string playHotkey: ""
    property string assignHotkey: ""
    property bool isLocked: false

    signal rebindRequested()

    Layout.fillWidth: true
    spacing: 4

    SaikoSectionLabel {
        text: "HOTKEYS"
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 48
        color: Theme.inputBackground
        border.color: Theme.borderDefault
        radius: 6

        Timer {
            id: cycleTimer
            interval: 3000
            running: true
            repeat: true
            onTriggered: hotkeyContainer.state = (hotkeyContainer.state === "showAssign") ? "showPlay" : "showAssign"
        }

        RowLayout {
            anchors { fill: parent; leftMargin: 12; rightMargin: 8 }
            spacing: 8

            Image {
                source: "image://icons/keyboard?color=%23b0b0b0"
                smooth: true
                sourceSize: Qt.size(18, 18)
            }

            Item {
                id: hotkeyContainer
                Layout.fillWidth: true
                implicitHeight: 24
                clip: true
                state: "showPlay"

                RowLayout {
                    id: playRow
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 6
                    opacity: 1.0

                    Text {
                        text: "Play Action:"
                        font.pixelSize: 12
                        color: Theme.textSecondary
                    }

                    Rectangle {
                        implicitWidth: playText.implicitWidth + 12
                        implicitHeight: 20
                        color: "#2a2a2a"
                        border.color: "#3a3a3a"
                        radius: 4
                        Text {
                            id: playText
                            anchors.centerIn: parent
                            text: root.playHotkey || "—"
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                            color: "#e0e0e0"
                        }
                    }
                }

                RowLayout {
                    id: assignRow
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 6
                    opacity: 0.0

                    Text {
                        text: "Assign from Replay Action:"
                        font.pixelSize: 12
                        color: Theme.textSecondary
                    }

                    Rectangle {
                        implicitWidth: assignText.implicitWidth + 12
                        implicitHeight: 20
                        color: "#2a2a2a"
                        border.color: "#3a3a3a"
                        radius: 4
                        Text {
                            id: assignText
                            anchors.centerIn: parent
                            text: root.assignHotkey || "—"
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                            color: "#e0e0e0"
                        }
                    }
                }

                states: [
                    State {
                        name: "showPlay"
                        PropertyChanges { target: playRow; opacity: 1.0 }
                        PropertyChanges { target: assignRow; opacity: 0.0 }
                    },
                    State {
                        name: "showAssign"
                        PropertyChanges { target: playRow; opacity: 0.0 }
                        PropertyChanges { target: assignRow; opacity: 1.0 }
                    }
                ]

                transitions: Transition {
                    NumberAnimation { property: "opacity"; duration: 250; easing.type: Easing.InOutQuad }
                }
            }

            SaikoIconButton {
                isActive: !root.isLocked
                tooltipText: "Rebind"
                iconSource: "image://icons/key-round?color=%23b0b0b0"
                onClicked: root.rebindRequested()
            }
        }
    }
}
