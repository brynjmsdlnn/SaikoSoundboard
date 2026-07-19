import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

SettingsPanelWrapper {
    title: "Notifications"
    subtitle: "Configure toast alerts and overlay settings."

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: cardLayout.implicitHeight + 32
            color: Theme.cardBackground
            radius: Theme.cardRadius
            border.color: Theme.borderDefault
            border.width: 1

            ColumnLayout {
                id: cardLayout
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    SaikoSectionLabel {
                        text: "ALERT PREFERENCES"
                    }
                    Text {
                        text: "Show toast notifications when you trigger hotkeys or finish audio recordings. The overlay stays visible on top of other windows/games."
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSizeNormal
                        wrapMode: Text.WordWrap
                        lineHeight: 1.15
                        Layout.fillWidth: true
                    }
                }

                SaikoCheckBox {
                    id: enabledCheckbox
                    text: "Enable Notifications"
                    checked: Backend.notifications.enabled
                    onCheckedChanged: {
                        Backend.notifications.enabled = checked
                        Backend.settings.save()
                    }
                }

                SaikoCheckBox {
                    id: overlayCheckbox
                    text: "Enable Overlay Mode (Always On Top)"
                    checked: Backend.notifications.overlayEnabled
                    enabled: enabledCheckbox.checked
                    onCheckedChanged: {
                        Backend.notifications.overlayEnabled = checked
                        Backend.settings.save()
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6
                    enabled: enabledCheckbox.checked

                    Text {
                        text: "Notification Duration"
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeNormal
                        font.weight: Font.DemiBold
                    }

                    SaikoComboBox {
                        id: durationCombo
                        Layout.fillWidth: true
                        model: [
                            { text: "1.5 seconds", value: 1500 },
                            { text: "3.0 seconds (Default)", value: 3000 },
                            { text: "5.0 seconds", value: 5000 },
                            { text: "8.0 seconds", value: 8000 }
                        ]
                        textRole: "text"
                        valueRole: "value"
                        onActivated: function(index) {
                            Backend.notifications.durationMs = model[index].value
                            Backend.settings.save()
                        }
                        Component.onCompleted: {
                            var idx = indexOfValue(Backend.notifications.durationMs)
                            if (idx >= 0) currentIndex = idx
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6
                    enabled: enabledCheckbox.checked

                    Text {
                        text: "Screen Position"
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeNormal
                        font.weight: Font.DemiBold
                    }

                    SaikoComboBox {
                        id: positionCombo
                        Layout.fillWidth: true
                        model: [
                            { text: "Bottom Right", value: "BottomRight" },
                            { text: "Bottom Left", value: "BottomLeft" },
                            { text: "Top Right", value: "TopRight" },
                            { text: "Top Left", value: "TopLeft" }
                        ]
                        textRole: "text"
                        valueRole: "value"
                        onActivated: function(index) {
                            Backend.notifications.position = model[index].value
                            Backend.settings.save()
                        }
                        Component.onCompleted: {
                            var idx = indexOfValue(Backend.notifications.position)
                            if (idx >= 0) currentIndex = idx
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6
                    enabled: enabledCheckbox.checked

                    Text {
                        text: "Notification Size"
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeNormal
                        font.weight: Font.DemiBold
                    }

                    SaikoComboBox {
                        id: sizeCombo
                        Layout.fillWidth: true
                        model: [
                            { text: "Extra Small", value: "ExtraSmall" },
                            { text: "Small", value: "Small" },
                            { text: "Medium (Default)", value: "Medium" },
                            { text: "Large", value: "Large" },
                            { text: "Extra Large", value: "ExtraLarge" }
                        ]
                        textRole: "text"
                        valueRole: "value"
                        onActivated: function(index) {
                            Backend.notifications.size = model[index].value
                            Backend.settings.save()
                        }
                        Component.onCompleted: {
                            var idx = indexOfValue(Backend.notifications.size)
                            if (idx >= 0) currentIndex = idx
                        }
                    }
                }

                // Preview button — fires a test notification to let users see
                // the current size, position, and duration in real time.
                SaikoButton {
                    id: previewBtn
                    Layout.fillWidth: true
                    Layout.topMargin: 4
                    text: "Preview Notification"
                    filled: true
                    accentColor: Theme.accentPurple
                    enabled: enabledCheckbox.checked
                    onClicked: {
                        Backend.notifications.postNotification(
                            "Saiko Soundboard",
                            "Preview — your settings look great!",
                            "info"
                        );
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
