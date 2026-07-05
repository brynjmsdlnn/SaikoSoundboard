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

    property string slotId: ""
    property string playKey: "SPACE"
    property string assignKey: "CTRL+A"
    property string activeField: ""

    property string initialPlayKey: ""
    property string initialAssignKey: ""

    signal accepted()
    signal rejected()

    function updateUnsavedFlags() {
        playCard.hasUnsavedChanges = (root.playKey !== root.initialPlayKey)
        assignCard.hasUnsavedChanges = (root.assignKey !== root.initialAssignKey)
    }

    function isDuplicate(seq, isPlayAction) {
        if (seq === "") return false
        var otherKey = isPlayAction ? root.assignKey : root.playKey
        var ownKey = isPlayAction ? root.playKey : root.assignKey
        if (seq === otherKey) return true
        if (seq === ownKey) return false
        for (var i = 0; i < SlotModel.rowCount(); i++) {
            var item = SlotModel.get(i)
            if (item.slotId === root.slotId) continue
            if (seq === item.playHotkey || seq === item.assignHotkey) return true
        }
        return false
    }

    property bool hotkeysWereOn: true

    function restoreHotkeys() {
        Backend.settings.hotkeysEnabled = hotkeysWereOn
    }

    Component.onCompleted: {
        initialPlayKey = root.playKey
        initialAssignKey = root.assignKey
        hotkeysWereOn = Backend.settings.hotkeysEnabled
        Backend.settings.hotkeysEnabled = false
    }

    Component.onDestruction: {
        restoreHotkeys()
    }

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
                id: playCard
                title: "PLAY ACTION"
                currentKey: root.playKey
                isRecording: root.activeField === "play"
                accentColor: Theme.accentPurple
                onClicked: root.activeField = "play"
                onKeyCaptured: (seq) => {
                    if (root.isDuplicate(seq, true)) {
                        playCard.showErrorWithMessage("DUPLICATE")
                    } else {
                        root.playKey = seq; root.activeField = ""
                        root.updateUnsavedFlags()
                    }
                }
                onCaptureFailed: root.activeField = ""
            }
            HotkeyCard {
                id: assignCard
                title: "ASSIGN ACTION"
                currentKey: root.assignKey
                isRecording: root.activeField === "assign"
                accentColor: Theme.accentTeal
                onClicked: root.activeField = "assign"
                onKeyCaptured: (seq) => {
                    if (root.isDuplicate(seq, false)) {
                        assignCard.showErrorWithMessage("DUPLICATE")
                    } else {
                        root.assignKey = seq; root.activeField = ""
                        root.updateUnsavedFlags()
                    }
                }
                onCaptureFailed: root.activeField = ""
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            SaikoButton {
                text: "Cancel"
                Layout.fillWidth: true
                onClicked: { root.restoreHotkeys(); root.rejected() }
            }

            SaikoButton {
                text: "Save bindings"
                Layout.fillWidth: true
                accentColor: Theme.accentPurple
                onClicked: { root.restoreHotkeys(); root.accepted() }
            }
        }
    }


}