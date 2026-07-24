import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0
import "../components"

Popup {
    id: dialog
    parent: Overlay.overlay
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    width: 420
    modal: true
    dim: true
    closePolicy: Popup.NoAutoClose

    padding: 24

    background: Rectangle {
        color: Theme.cardBackground
        border.color: Theme.borderDefault
        border.width: 1
        radius: Theme.cardRadius

        Rectangle {
            anchors.fill: parent
            anchors.margins: -1
            color: "transparent"
            border.color: Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.15)
            border.width: 1
            radius: Theme.cardRadius + 1
        }
    }

    ColumnLayout {
        width: parent.width
        spacing: 16

        // Title
        Text {
            text: "Close Saiko Soundboard?"
            font.pixelSize: 18
            font.weight: Font.Bold
            color: Theme.textPrimary
            Layout.fillWidth: true
        }

        // Subtitle / Description
        Text {
            text: "Saiko Soundboard can continue running in the background so replay buffer, hotkeys, and notifications remain active."
            font.pixelSize: 13
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            lineHeight: 1.2
            Layout.fillWidth: true
        }

        // Feature bullet list
        ColumnLayout {
            spacing: 6
            Layout.fillWidth: true
            Layout.leftMargin: 8

            Text {
                text: "• Replay buffer & clip saving"
                font.pixelSize: 12
                color: Theme.textDim
            }
            Text {
                text: "• Global soundboard hotkeys"
                font.pixelSize: 12
                color: Theme.textDim
            }
            Text {
                text: "• Toast notifications overlay"
                font.pixelSize: 12
                color: Theme.textDim
            }
        }

        // Remember my choice checkbox
        SaikoCheckBox {
            id: rememberCheckBox
            text: "Remember my choice"
            checked: false
            Layout.topMargin: 4
        }

        Item { Layout.preferredHeight: 4 }

        // Action buttons
        ColumnLayout {
            spacing: 8
            Layout.fillWidth: true

            SaikoButton {
                text: "Minimize to System Tray (Recommended)"
                Layout.fillWidth: true
                filled: true
                accentColor: Theme.accentPurple
                onClicked: {
                    dialog.close()
                    Backend.lifecycle.setCloseChoice(SettingsManager.MinimizeToTray, rememberCheckBox.checked)
                }
            }

            SaikoButton {
                text: "Exit Application"
                Layout.fillWidth: true
                accentColor: Theme.destructiveRed
                onClicked: {
                    dialog.close()
                    Backend.lifecycle.setCloseChoice(SettingsManager.Exit, rememberCheckBox.checked)
                }
            }
        }
    }
}
