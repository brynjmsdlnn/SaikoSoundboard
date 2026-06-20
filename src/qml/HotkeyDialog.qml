import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Saiko 1.0

Window {
    id: root
    width: 520
    height: 340
    minimumWidth: 520
    minimumHeight: 340
    maximumWidth: 520
    maximumHeight: 340
    color: Theme.appBackground
    title: "Hotkey Configuration"
    modality: Qt.ApplicationModal
    flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint

    property string playKey: "SPACE"
    property string assignKey: "CTRL+A"
    property string activeField: ""

    signal accepted()
    signal rejected()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 20

        ColumnLayout {
            spacing: 4
            Text {
                text: "Configure Controls"
                color: Theme.textPrimary
                font.pixelSize: 22
                font.weight: Font.Bold
            }
            Text {
                text: "Click a card below to rebind the key combination."
                color: Theme.textDim
                font.pixelSize: Theme.fontSizeHeading
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            HotkeyCard {
                title: "PLAY ACTION"
                currentKey: root.playKey
                isRecording: root.activeField === "play"
                accentColor: Theme.accentPurple
                onClicked: root.activeField = "play"
                onKeyCaptured: (seq) => { root.playKey = seq; root.activeField = "" }
            }
            HotkeyCard {
                title: "ASSIGN ACTION"
                currentKey: root.assignKey
                isRecording: root.activeField === "assign"
                accentColor: Theme.accentTeal
                onClicked: root.activeField = "assign"
                onKeyCaptured: (seq) => { root.assignKey = seq; root.activeField = "" }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            ThemedButton {
                text: "Cancel"
                Layout.fillWidth: true
                onClicked: root.rejected()
            }

            ThemedButton {
                text: "Save bindings"
                Layout.fillWidth: true
                accentColor: Theme.accentPurple
                onClicked: root.accepted()
            }
        }
    }


}