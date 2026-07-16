import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

SettingsPanelWrapper {
    title: "Storage Settings"
    subtitle: "Manage where recordings and replays are saved."

    signal changeBaseRequested()
    signal changeRecordingRequested()
    signal changeReplayRequested()
    signal resetRecordingRequested()
    signal resetReplayRequested()

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: storageLayout.implicitHeight + 32
            color: Theme.cardBackground
            radius: Theme.cardRadius || 8
            border.color: Theme.borderDefault
            border.width: 1

            ColumnLayout {
                id: storageLayout
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    SaikoSectionLabel {
                        text: "STORAGE LOCATIONS"
                    }
                    Text {
                        text: "By default, recordings and replays are saved inside the Base Directory. You can override each directory to point elsewhere."
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSizeNormal || 13
                        wrapMode: Text.WordWrap
                        lineHeight: 1.15
                        Layout.fillWidth: true
                    }
                }

                // Base Directory
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6
                    Text {
                        text: "Base Directory (Default Root)"
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeNormal || 13
                        font.weight: Font.DemiBold
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        Rectangle {
                            Layout.fillWidth: true
                            height: 36
                            color: Theme.appBackground
                            radius: Theme.borderRadius || 6
                            border.color: Theme.borderDefault
                            border.width: 1
                            clip: true

                            Label {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                verticalAlignment: Text.AlignVCenter
                                color: Theme.textPrimary
                                elide: Text.ElideMiddle
                                text: Backend.settings.baseDirectory
                                font.pixelSize: Theme.fontSizeNormal || 13
                                ToolTip { text: parent.text; visible: parent.truncated; delay: 600 }
                            }
                        }
                        SaikoButton {
                            text: "Change\u2026"
                            implicitHeight: 36
                            onClicked: changeBaseRequested()
                        }
                    }
                }

                // Recordings Override
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6
                    RowLayout {
                        spacing: 8
                        Text {
                            text: "Recordings Directory"
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSizeNormal || 13
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: Backend.settings.recordingDirectoryOverride ? "(Custom Override)" : "(Default)"
                            color: Backend.settings.recordingDirectoryOverride ? (Theme.accentPurple || "#BB86FC") : Theme.textDim
                            font.pixelSize: 11
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        Rectangle {
                            Layout.fillWidth: true
                            height: 36
                            color: Theme.appBackground
                            radius: Theme.borderRadius || 6
                            border.color: Theme.borderDefault
                            border.width: 1
                            clip: true

                            Label {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                verticalAlignment: Text.AlignVCenter
                                color: Theme.textPrimary
                                elide: Text.ElideMiddle
                                text: Backend.settings.recordingDirectory
                                font.pixelSize: Theme.fontSizeNormal || 13
                                ToolTip { text: parent.text; visible: parent.truncated; delay: 600 }
                            }
                        }
                        SaikoButton {
                            text: "Change\u2026"
                            implicitHeight: 36
                            onClicked: changeRecordingRequested()
                        }
                        SaikoButton {
                            text: "Reset"
                            implicitHeight: 36
                            enabled: !!Backend.settings.recordingDirectoryOverride
                            onClicked: resetRecordingRequested()
                        }
                    }
                }

                // Replays Override
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6
                    RowLayout {
                        spacing: 8
                        Text {
                            text: "Replays Directory"
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSizeNormal || 13
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: Backend.settings.replayDirectoryOverride ? "(Custom Override)" : "(Default)"
                            color: Backend.settings.replayDirectoryOverride ? (Theme.accentPurple || "#BB86FC") : Theme.textDim
                            font.pixelSize: 11
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        Rectangle {
                            Layout.fillWidth: true
                            height: 36
                            color: Theme.appBackground
                            radius: Theme.borderRadius || 6
                            border.color: Theme.borderDefault
                            border.width: 1
                            clip: true

                            Label {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                verticalAlignment: Text.AlignVCenter
                                color: Theme.textPrimary
                                elide: Text.ElideMiddle
                                text: Backend.settings.replayDirectory
                                font.pixelSize: Theme.fontSizeNormal || 13
                                ToolTip { text: parent.text; visible: parent.truncated; delay: 600 }
                            }
                        }
                        SaikoButton {
                            text: "Change\u2026"
                            implicitHeight: 36
                            onClicked: changeReplayRequested()
                        }
                        SaikoButton {
                            text: "Reset"
                            implicitHeight: 36
                            enabled: !!Backend.settings.replayDirectoryOverride
                            onClicked: resetReplayRequested()
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
