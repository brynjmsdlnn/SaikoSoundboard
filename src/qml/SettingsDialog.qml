import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Saiko 1.0

Window {
    id: root
    width: 520
    height: 380
    minimumWidth: 520
    minimumHeight: 380
    maximumWidth: 520
    maximumHeight: 380
    color: Theme.appBackground
    title: "Storage Settings"
    modality: Qt.ApplicationModal
    flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint

    signal changeBaseRequested()
    signal changeRecordingRequested()
    signal changeReplayRequested()
    signal resetRecordingRequested()
    signal resetReplayRequested()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        ColumnLayout {
            spacing: 4
            Text {
                text: "Storage Folders"
                color: Theme.textPrimary
                font.pixelSize: 18
                font.weight: Font.Bold
            }
            Text {
                text: "By default, recordings and replays are saved inside the Base Directory. You can override each directory to point elsewhere."
                color: Theme.textDim
                font.pixelSize: Theme.fontSizeNormal
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        // Section: Base Directory
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6
            Text {
                text: "Base Directory (Default Root)"
                color: Theme.textSecondary
                font.pixelSize: 12
                font.bold: true
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Rectangle {
                    Layout.fillWidth: true
                    height: 28
                    color: Theme.recessedBackground
                    radius: Theme.borderRadius
                    border.color: Theme.borderDefault
                    Label {
                        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        color: Theme.textPrimary
                        elide: Text.ElideMiddle
                        text: Backend.settings.baseDirectory
                        ToolTip { text: parent.text; visible: parent.truncated; delay: 600 }
                    }
                }
                ThemedButton {
                    text: "Change\u2026"
                    small: true
                    onClicked: root.changeBaseRequested()
                }
            }
        }

        // Section: Recordings Override
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6
            RowLayout {
                spacing: 6
                Text {
                    text: "Recordings Directory"
                    color: Theme.textSecondary
                    font.pixelSize: 12
                    font.bold: true
                }
                Text {
                    text: Backend.settings.recordingDirectoryOverride ? "(Custom Override)" : "(Default)"
                    color: Backend.settings.recordingDirectoryOverride ? Theme.accentPurple : Theme.textDim
                    font.pixelSize: 11
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Rectangle {
                    Layout.fillWidth: true
                    height: 28
                    color: Theme.recessedBackground
                    radius: Theme.borderRadius
                    border.color: Theme.borderDefault
                    Label {
                        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        color: Theme.textPrimary
                        elide: Text.ElideMiddle
                        text: Backend.settings.recordingDirectory
                        ToolTip { text: parent.text; visible: parent.truncated; delay: 600 }
                    }
                }
                ThemedButton {
                    text: "Change\u2026"
                    small: true
                    onClicked: root.changeRecordingRequested()
                }
                ThemedButton {
                    text: "Reset"
                    small: true
                    enabled: !!Backend.settings.recordingDirectoryOverride
                    onClicked: root.resetRecordingRequested()
                }
            }
        }

        // Section: Replays Override
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6
            RowLayout {
                spacing: 6
                Text {
                    text: "Replays Directory"
                    color: Theme.textSecondary
                    font.pixelSize: 12
                    font.bold: true
                }
                Text {
                    text: Backend.settings.replayDirectoryOverride ? "(Custom Override)" : "(Default)"
                    color: Backend.settings.replayDirectoryOverride ? Theme.accentPurple : Theme.textDim
                    font.pixelSize: 11
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Rectangle {
                    Layout.fillWidth: true
                    height: 28
                    color: Theme.recessedBackground
                    radius: Theme.borderRadius
                    border.color: Theme.borderDefault
                    Label {
                        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        color: Theme.textPrimary
                        elide: Text.ElideMiddle
                        text: Backend.settings.replayDirectory
                        ToolTip { text: parent.text; visible: parent.truncated; delay: 600 }
                    }
                }
                ThemedButton {
                    text: "Change\u2026"
                    small: true
                    onClicked: root.changeReplayRequested()
                }
                ThemedButton {
                    text: "Reset"
                    small: true
                    enabled: !!Backend.settings.replayDirectoryOverride
                    onClicked: root.resetReplayRequested()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            ThemedButton {
                text: "Close"
                small: true
                onClicked: root.close()
            }
        }
    }
}
