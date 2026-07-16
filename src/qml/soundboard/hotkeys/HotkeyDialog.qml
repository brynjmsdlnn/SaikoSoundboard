import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

SaikoFramelessPopup {
    id: root
    width: 520
    height: 380

    property string slotId: ""
    property string playKey: "SPACE"
    property string assignKey: "CTRL+A"
    property string activeField: ""

    property string initialPlayKey: ""
    property string initialAssignKey: ""

    signal accepted()
    signal rejected()

    property bool hotkeysWereOn: true

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

    function restoreHotkeys() {
        Backend.settings.hotkeysEnabled = hotkeysWereOn
    }

    function openForSlot(slotId, playKey, assignKey) {
        root.slotId = slotId
        root.playKey = playKey
        root.assignKey = assignKey
        root.initialPlayKey = playKey
        root.initialAssignKey = assignKey
        root.activeField = ""
        root.hotkeysWereOn = Backend.settings.hotkeysEnabled
        Backend.settings.hotkeysEnabled = false
        root.updateUnsavedFlags()
        root.open()
    }

    onClosed: {
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
                onClicked: { root.restoreHotkeys(); root.rejected(); root.close() }
            }

            SaikoButton {
                text: "Save bindings"
                Layout.fillWidth: true
                accentColor: Theme.accentPurple
                onClicked: { root.restoreHotkeys(); root.accepted(); root.close() }
            }
        }
    }
}
