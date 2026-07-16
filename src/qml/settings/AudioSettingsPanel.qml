import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

SettingsPanelWrapper {
    title: "Audio Settings"
    subtitle: "Configure recording quality and options."

    readonly property var sampleRateModel: [
        { text: "System Default (" + Backend.systemDefaultSampleRate() + " Hz)", value: 0 },
        { text: "22,050 Hz", value: 22050 },
        { text: "44,100 Hz", value: 44100 },
        { text: "48,000 Hz", value: 48000 },
        { text: "96,000 Hz", value: 96000 }
    ]

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

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
                    SaikoSectionLabel {
                        text: "AUDIO QUALITY"
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
                    SaikoComboBox {
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

        Item { Layout.fillHeight: true }
    }
}
