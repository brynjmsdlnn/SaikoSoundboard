import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Saiko 1.0

Window {
    id: root
    width: 480
    height: 520
    minimumWidth: 460
    minimumHeight: 500
    maximumWidth: 540
    maximumHeight: 560
    color: Theme.appBackground
    title: "About SaikoSoundboard"
    modality: Qt.ApplicationModal
    flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // --- App Header & Logo Badge ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            Rectangle {
                width: 60
                height: 60
                radius: Theme.cardRadius
                color: Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.15)
                border.color: Theme.accentPurple
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "((\u2022))"
                    color: Theme.accentPurple
                    font.pixelSize: 22
                    font.bold: true
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                RowLayout {
                    spacing: 8
                    Text {
                        text: "SaikoSoundboard"
                        color: Theme.textPrimary
                        font.pixelSize: 22
                        font.weight: Font.Bold
                    }
                    Rectangle {
                        height: 20
                        implicitWidth: versionText.implicitWidth + 12
                        color: Qt.rgba(Theme.accentTeal.r, Theme.accentTeal.g, Theme.accentTeal.b, 0.2)
                        border.color: Theme.accentTeal
                        border.width: 1
                        radius: 10

                        Text {
                            id: versionText
                            anchors.centerIn: parent
                            text: "v0.2.0 Beta"
                            color: Theme.accentTeal
                            font.pixelSize: 11
                            font.weight: Font.Bold
                        }
                    }
                }

                Text {
                    text: "High-Performance Windows Audio Capture & Soundboard"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeNormal
                }
            }
        }

        // --- Technical Information Card ---
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: techLayout.implicitHeight + 24
            color: Theme.cardBackground
            radius: Theme.cardRadius
            border.color: Theme.borderDefault
            border.width: 1

            ColumnLayout {
                id: techLayout
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                SaikoSectionLabel {
                    text: "TECHNICAL SPECIFICATIONS"
                }

                GridLayout {
                    columns: 2
                    rowSpacing: 8
                    columnSpacing: 16
                    Layout.fillWidth: true

                    Text { text: "Audio Subsystem:"; color: Theme.textDim; font.pixelSize: Theme.fontSizeNormal }
                    Text { text: "WASAPI Low-Latency Loopback"; color: Theme.textPrimary; font.pixelSize: Theme.fontSizeNormal; font.weight: Font.DemiBold }

                    Text { text: "GUI Framework:"; color: Theme.textDim; font.pixelSize: Theme.fontSizeNormal }
                    Text { text: "Qt 6 Quick (QML)"; color: Theme.textPrimary; font.pixelSize: Theme.fontSizeNormal; font.weight: Font.DemiBold }

                    Text { text: "Core Engine:"; color: Theme.textDim; font.pixelSize: Theme.fontSizeNormal }
                    Text { text: "C++17 Multi-threaded Audio Mixer"; color: Theme.textPrimary; font.pixelSize: Theme.fontSizeNormal; font.weight: Font.DemiBold }

                    Text { text: "Hotkey Dispatcher:"; color: Theme.textDim; font.pixelSize: Theme.fontSizeNormal }
                    Text { text: "Native Windows Hook / Win32 API"; color: Theme.textPrimary; font.pixelSize: Theme.fontSizeNormal; font.weight: Font.DemiBold }
                }
            }
        }

        // --- Features Summary Card ---
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: featuresLayout.implicitHeight + 24
            color: Theme.cardBackground
            radius: Theme.cardRadius
            border.color: Theme.borderDefault
            border.width: 1

            ColumnLayout {
                id: featuresLayout
                anchors.fill: parent
                anchors.margins: 16
                spacing: 8

                SaikoSectionLabel {
                    text: "CORE CAPABILITIES"
                }

                Text {
                    text: "\u2022 Global Windows Desktop Loopback Audio Capture\n\u2022 Per-Process Audio Stream Isolation & Multi-Track Recording\n\u2022 Instant Replay Buffer with Configurable Time Window\n\u2022 Low-Latency Soundboard Triggering with Custom Device Routing"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeNormal
                    lineHeight: 1.3
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        Item { Layout.fillHeight: true }

        // --- Bottom Action Row ---
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "\u00a9 2026 Saiko Interactive"
                color: Theme.textDim
                font.pixelSize: Theme.fontSizeSmall
            }

            Item { Layout.fillWidth: true }

            SaikoButton {
                text: "Close"
                implicitWidth: 90
                implicitHeight: 32
                onClicked: root.close()
            }
        }
    }
}
