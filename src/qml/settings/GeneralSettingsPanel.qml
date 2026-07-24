import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

SettingsPanelWrapper {
    title: "General Settings"
    subtitle: "Configure application behavior and startup preferences."

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        // Window Close Behavior Card
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: windowBehaviorLayout.implicitHeight + 32
            color: Theme.cardBackground
            radius: Theme.cardRadius || 8
            border.color: Theme.borderDefault
            border.width: 1

            ColumnLayout {
                id: windowBehaviorLayout
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    SaikoSectionLabel {
                        text: "WINDOW BEHAVIOR"
                    }
                    Text {
                        text: "Control what happens when you press the close (X) button on the main window."
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSizeNormal || 13
                        wrapMode: Text.WordWrap
                        lineHeight: 1.15
                        Layout.fillWidth: true
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Text {
                            text: "When closing main window:"
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSizeNormal || 13
                            font.weight: Font.DemiBold
                        }
                    }

                    SaikoComboBox {
                        id: closeBehaviorCombo
                        implicitWidth: 220
                        model: [
                            { text: "Minimize to system tray", value: SettingsManager.MinimizeToTray },
                            { text: "Exit application", value: SettingsManager.Exit },
                            { text: "Always ask", value: SettingsManager.Ask }
                        ]

                        Component.onCompleted: updateCurrentIndex()

                        Connections {
                            target: Backend.settings
                            function onCloseBehaviorChanged() {
                                closeBehaviorCombo.updateCurrentIndex()
                            }
                        }

                        function updateCurrentIndex() {
                            if (!model || !model.length || !Backend.settings) return
                            var current = Backend.settings.closeBehavior
                            for (var i = 0; i < model.length; i++) {
                                if (model[i].value === current) {
                                    currentIndex = i
                                    return
                                }
                            }
                            currentIndex = 2 // Always ask
                        }

                        onActivated: function(index) {
                            var val = model[index].value
                            Backend.settings.setCloseBehavior(val)
                        }
                    }
                }
            }
        }

        // Tray Icon Color Card
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: trayIconLayout.implicitHeight + 32
            color: Theme.cardBackground
            radius: Theme.cardRadius || 8
            border.color: Theme.borderDefault
            border.width: 1

            ColumnLayout {
                id: trayIconLayout
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    SaikoSectionLabel {
                        text: "TRAY ICON"
                    }
                    Text {
                        text: "Customize the system tray icon color."
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSizeNormal || 13
                        wrapMode: Text.WordWrap
                        lineHeight: 1.15
                        Layout.fillWidth: true
                    }
                }

                // Color swatches + live preview row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    // Live preview
                    Rectangle {
                        id: previewCircle
                        implicitWidth: 40
                        implicitHeight: 40
                        radius: 20
                        border.color: Theme.borderDefault
                        border.width: 2
                        color: Backend.settings.trayIconColor

                        Behavior on color {
                            ColorAnimation { duration: Theme.animDuration || 150 }
                        }

                        Image {
                            anchors.centerIn: parent
                            source: "image://icons/radio?color=%23ffffff"
                            sourceSize: Qt.size(20, 20)
                            width: 20
                            height: 20
                            visible: previewCircle.color !== "#ffffff" && previewCircle.color !== "#e8e8e8"
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        // Preset color swatches
                        Flow {
                            Layout.fillWidth: true
                            spacing: 6

                            readonly property var colors: [
                                "#e35d5d", // red
                                "#e8824a", // orange
                                "#e8b84a", // amber
                                "#5de37a", // green
                                "#4ad4e8", // cyan
                                "#5d7ae3", // blue
                                "#a855f7", // purple
                                "#e84a8a", // pink
                                "#ffffff", // white
                                "#94a3b8"  // gray
                            ]

                            Repeater {
                                model: parent.colors

                                delegate: Rectangle {
                                    width: 28
                                    height: 28
                                    radius: 14
                                    color: modelData
                                    border.width: Backend.settings.trayIconColor === modelData ? 2 : 1
                                    border.color: Backend.settings.trayIconColor === modelData ? (modelData === "#ffffff" ? Theme.accentPurple : "#ffffff") : Theme.borderDefault

                                    Behavior on border.width {
                                        NumberAnimation { duration: 100 }
                                    }

                                    Behavior on border.color {
                                        ColorAnimation { duration: 100 }
                                    }

                                    // Selected dot indicator
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: modelData === "#ffffff" || modelData === "#e8e8e8" ? Theme.textSecondary : "#ffffff"
                                        visible: Backend.settings.trayIconColor === modelData
                                        opacity: 0.8
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        hoverEnabled: true
                                        onHoveredChanged: {
                                            parent.scale = containsMouse ? 1.25 : 1.0
                                        }
                                        onClicked: {
                                            Backend.settings.setTrayIconColor(modelData)
                                        }
                                    }

                                    Behavior on scale {
                                        NumberAnimation { duration: 120; easing.type: Easing.OutBack }
                                    }
                                }
                            }
                        }

                        // Custom hex color input row
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: "Custom:"
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSizeNormal || 13
                                Layout.alignment: Qt.AlignVCenter
                            }

                            Rectangle {
                                id: hexInputBg
                                implicitWidth: 120
                                height: 32
                                color: Theme.appBackground
                                radius: Theme.borderRadius || 6
                                border.color: hexInput.activeFocus ? Theme.accentPurple : Theme.borderDefault
                                border.width: 1

                                Behavior on border.color {
                                    ColorAnimation { duration: Theme.animDuration || 120 }
                                }

                                TextInput {
                                    id: hexInput
                                    anchors.fill: parent
                                    anchors.leftMargin: 10
                                    anchors.rightMargin: 10
                                    verticalAlignment: TextInput.AlignVCenter
                                    color: Theme.textPrimary
                                    font.pixelSize: Theme.fontSizeNormal || 13
                                    font.family: "monospace"
                                    maximumLength: 7
                                    text: Backend.settings.trayIconColor

                                    validator: RegularExpressionValidator {
                                        regularExpression: /^#[0-9a-fA-F]{0,6}$/
                                    }

                                    onEditingFinished: {
                                        var t = text.trim()
                                        if (t.length === 7 && /^#[0-9a-fA-F]{6}$/.test(t)) {
                                            Backend.settings.setTrayIconColor(t)
                                        } else {
                                            // Revert to current value
                                            text = Backend.settings.trayIconColor
                                        }
                                    }

                                    onActiveFocusChanged: {
                                        if (!activeFocus) {
                                            var t = text.trim()
                                            if (t.length === 7 && /^#[0-9a-fA-F]{6}$/.test(t)) {
                                                Backend.settings.setTrayIconColor(t)
                                            } else {
                                                text = Backend.settings.trayIconColor
                                            }
                                        }
                                    }
                                }
                            }

                            // Tiny preview of custom color
                            Rectangle {
                                implicitWidth: 24
                                implicitHeight: 24
                                radius: 12
                                color: hexInput.text
                                border.color: Theme.borderDefault
                                border.width: 1
                            }
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
