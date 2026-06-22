import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Saiko 1.0

Window {
    id: root
    width: 540
    height: 610
    minimumWidth: 520
    minimumHeight: 600
    maximumWidth: 600
    maximumHeight: 800
    color: Theme.appBackground
    title: "Storage & Audio Settings"
    modality: Qt.ApplicationModal
    flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint

    signal changeBaseRequested()
    signal changeRecordingRequested()
    signal changeReplayRequested()
    signal resetRecordingRequested()
    signal resetReplayRequested()

    readonly property var sampleRateModel: [
        { text: "System Default (" + Backend.systemDefaultSampleRate() + " Hz)", value: 0 },
        { text: "22,050 Hz", value: 22050 },
        { text: "44,100 Hz", value: 44100 },
        { text: "48,000 Hz", value: 48000 },
        { text: "96,000 Hz", value: 96000 }
    ]

    ColumnLayout {
        id: mainColumn
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // --- Title Header ---
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                Text {
                    text: "Storage & Audio Settings"
                    color: Theme.textPrimary
                    font.pixelSize: 22
                    font.weight: Font.Bold
                }
                Text {
                    text: "Manage where files are saved and set recording quality."
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSizeHeading || 14
                }
            }

            // --- Storage Folders Card ---
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
                        Text {
                            text: "STORAGE LOCATIONS"
                            color: Theme.textDim
                            font.pixelSize: Theme.fontSizeSmall || 11
                            font.letterSpacing: 1.5
                            font.weight: Font.Bold
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
                                color: Theme.appBackground // Using app background to look like a recessed input field
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
                            ThemedButton {
                                text: "Change\u2026"
                                implicitHeight: 36
                                onClicked: root.changeBaseRequested()
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
                            ThemedButton {
                                text: "Change\u2026"
                                implicitHeight: 36
                                onClicked: root.changeRecordingRequested()
                            }
                            ThemedButton {
                                text: "Reset"
                                implicitHeight: 36
                                enabled: !!Backend.settings.recordingDirectoryOverride
                                onClicked: root.resetRecordingRequested()
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
                            ThemedButton {
                                text: "Change\u2026"
                                implicitHeight: 36
                                onClicked: root.changeReplayRequested()
                            }
                            ThemedButton {
                                text: "Reset"
                                implicitHeight: 36
                                enabled: !!Backend.settings.replayDirectoryOverride
                                onClicked: root.resetReplayRequested()
                            }
                        }
                    }
                }
            }

            // --- Recording Quality Card ---
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: qualityLayout.implicitHeight + 32
                color: Theme.cardBackground
                radius: Theme.cardRadius || 8
                border.color: Theme.borderDefault
                border.width: 1

                ColumnLayout {
                    id: qualityLayout
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        Text {
                            text: "AUDIO QUALITY"
                            color: Theme.textDim
                            font.pixelSize: Theme.fontSizeSmall || 11
                            font.letterSpacing: 1.5
                            font.weight: Font.Bold
                        }
                        Text {
                            text: "Sample rate controls audio frequency range; higher rates capture more detail. Recordings are always saved as 32-bit float for maximum fidelity."
                            color: Theme.textDim
                            font.pixelSize: Theme.fontSizeNormal || 13
                            wrapMode: Text.WordWrap
                            lineHeight: 1.15
                            Layout.fillWidth: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Text {
                            text: "Sample Rate"
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSizeNormal || 13
                            font.weight: Font.DemiBold
                        }
                        CustomComboBox {
                            id: sampleRateCombo
                            Layout.fillWidth: true
                            model: sampleRateModel
                            textRole: "text"
                            valueRole: "value"
                            onActivated: function(index) {
                                Backend.settings.recordingSampleRate = model[index].value
                                Backend.settings.save()
                            }
                            Component.onCompleted: {
                                var idx = indexOfValue(Backend.settings.recordingSampleRate)
                                if (idx >= 0) currentIndex = idx
                            }
                        }
                    }
                }
            }

            // Note: Close button removed entirely to match standard OS modal behavior.
        }
}