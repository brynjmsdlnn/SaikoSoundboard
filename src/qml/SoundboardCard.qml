import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Dialogs
import Saiko 1.0
import "utils.js" as Utils

Rectangle {
    id: root
    width: 210
    height: cardColumn.implicitHeight + 28
    color: cardHover.hovered ? "#141414" : "#111111"
    radius: 12
    border.color: cardHover.hovered ? "#262626" : Theme.borderDefault
    border.width: 1

    Behavior on color { ColorAnimation { duration: 180 } }
    Behavior on border.color { ColorAnimation { duration: 180 } }

    HoverHandler {
        id: cardHover
    }

    property var slotModel: null
    property int slotIndex: -1

    // ============================================================
    // Card content
    // ============================================================
    ColumnLayout {
        id: cardColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 12
        spacing: 10

        // --- Header row: name + options trigger ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                Layout.fillWidth: true
                text: slotName || "Untitled"
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeHeading
                font.weight: Font.Medium
                elide: Text.ElideRight
                maximumLineCount: 1
            }

            Rectangle {
                id: optionsBtn
                width: 26
                height: 26
                radius: 7
                color: optionsMouse.containsMouse ? "#1f1f1f" : "transparent"
                border.color: optionsMouse.containsMouse ? "#2e2e2e" : "transparent"
                border.width: 1
                Behavior on color { ColorAnimation { duration: Theme.animDuration - 30 } }
                Behavior on border.color { ColorAnimation { duration: Theme.animDuration - 30 } }

                Text {
                    anchors.centerIn: parent
                    text: "\u22EF"
                    color: optionsMouse.containsMouse ? Theme.textPrimary : Theme.textDim
                    font.pixelSize: 15
                    Behavior on color { ColorAnimation { duration: Theme.animDuration - 30 } }
                }

                MouseArea {
                    id: optionsMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: optionsMenu.open()
                }

                CardMenu {
                    id: optionsMenu
                    y: optionsBtn.height + 4
                    x: optionsBtn.width - width

                    CardMenuItem {
                        text: "Rename"
                        onClicked: renameDialog.open()
                    }
                    CardMenuItem {
                        text: "Hotkey bindings"
                        onClicked: openHotkeyDialog()
                    }
                }
            }
        }

        // --- File status line ---
        Text {
            Layout.fillWidth: true
            text: {
                if (!filePath || filePath === "") return "No file assigned"
                var f = filePath.toString()
                var parts = f.split("/")
                var name = parts[parts.length - 1]
                return isTemporary ? "Temporary \u00b7 " + name : name
            }
            color: isTemporary ? "#d99a3d" : Theme.textDim
            font.pixelSize: Theme.fontSizeSmall
            elide: Text.ElideRight
            maximumLineCount: 1

            MouseArea {
                anchors.fill: parent
                cursorShape: isTemporary ? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: { if (isTemporary) makePermanentDialog.open() }
            }
        }

        // --- Waveform panel ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: Theme.recessedBackground
            radius: Theme.cardRadius
            border.color: Theme.borderDefault
            border.width: 1
            clip: true

            WaveformView {
                id: waveformView
                anchors.fill: parent
                anchors.margins: 3
                startMs: startTimeMs
                endMs: endTimeMs
                readOnly: false
                onTrimRangeChanged: (s, e) => {
                    if (slotModel && slotIndex >= 0) slotModel.setClipRange(slotIndex, s, e)
                }
                onTrimRangeCommit: (s, e) => {
                    if (slotModel && slotIndex >= 0) slotModel.setClipRange(slotIndex, s, e)
                }
            }
        }

        ClipRangeEditor {
            Layout.fillWidth: true
            startMs: startTimeMs
            endMs: endTimeMs
            durationSec: durationSec
            onClipRangeChanged: (s, e) => {
                if (slotModel && slotIndex >= 0) slotModel.setClipRange(slotIndex, s, e)
            }
        }

        // --- Volume + transport row ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Slider {
                id: volSlider
                orientation: Qt.Vertical
                Layout.preferredHeight: 76
                Layout.preferredWidth: 18
                Layout.alignment: Qt.AlignVCenter
                from: 0
                to: 100
                value: volume * 100
                live: true
                hoverEnabled: true

                background: Rectangle {
                    x: volSlider.leftPadding + volSlider.availableWidth / 2 - width / 2
                    y: volSlider.topPadding
                    width: 4
                    height: volSlider.availableHeight
                    radius: 2
                    color: Theme.borderDefault

                    Rectangle {
                        x: 0
                        y: parent.height * volSlider.visualPosition
                        width: parent.width
                        height: parent.height * (1.0 - volSlider.visualPosition)
                        radius: 2
                        color: Theme.accentPurple
                    }
                }

                handle: Rectangle {
                    x: volSlider.leftPadding + volSlider.availableWidth / 2 - width / 2
                    y: volSlider.topPadding + volSlider.visualPosition * volSlider.availableHeight - height / 2
                    width: 14
                    height: 14
                    radius: 7
                    color: volSlider.pressed ? Theme.accentPurple : (volSlider.hovered ? Theme.textPrimary : Theme.textDim)
                    border.color: volSlider.hovered ? Theme.accentPurple : "#555"
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: 100 } }
                    Behavior on border.color { ColorAnimation { duration: 100 } }
                }

                onMoved: {
                    if (slotModel && slotIndex >= 0) slotModel.setVolume(slotIndex, value / 100)
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    ThemedButton {
                        text: "Play"
                        accentColor: Theme.accentGreen
                        filled: true
                        small: true
                        onClicked: Backend.soundboard.playPlayer(slotId)
                    }
                    ThemedButton {
                        text: "Preview"
                        accentColor: Theme.accentTeal
                        filled: true
                        small: true
                        onClicked: Backend.soundboard.playPlayerPreview(slotId)
                    }
                }

                ThemedButton {
                    text: "Stop"
                    accentColor: Theme.accentRed
                    filled: true
                    small: true
                    onClicked: Backend.soundboard.stopPlayer(slotId)
                }
            }
        }

        // --- Routing selector ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "Route"
                color: Theme.textDim
                font.pixelSize: 9
                Layout.preferredWidth: 32
            }

            CustomComboBox {
                id: routingCombo
                Layout.fillWidth: true
                implicitHeight: 26
                font.pixelSize: 9
                radius: 6
                model: [
                    { text: "Broadcast & monitor", value: 0 },
                    { text: "Broadcast only", value: 1 },
                    { text: "Monitor only", value: 2 }
                ]
                textRole: "text"
                valueRole: "value"
                currentIndex: {
                    for (var i = 0; i < model.length; i++) {
                        if (model[i].value === outputRouting) return i
                    }
                    return 0
                }
                onCurrentValueChanged: {
                    if (slotModel && slotIndex >= 0) slotModel.setRouting(slotIndex, currentValue)
                }
            }
        }

        // --- Divider ---
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.borderDefault
        }

        // --- Footer: assign / remove ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            ThemedButton {
                id: assignBtn
                text: "Assign"
                small: true
                Layout.fillWidth: true
                onClicked: assignMenu.open()

                CardMenu {
                    id: assignMenu
                    y: assignBtn.height + 4
                    x: 0

                    CardMenuItem {
                        text: "From file..."
                        onClicked: assignFileDialog.open()
                    }
                    CardMenuItem {
                        text: "From replay buffer"
                        onClicked: Backend.actions.dispatchAssignReplay(slotId, preserveCb.checked)
                    }
                }
            }

            ThemedButton {
                id: removeBtn
                text: "\u2715"
                small: true
                accentColor: Theme.destructiveRed
                implicitWidth: 28
                onClicked: removeConfirmDialog.open()
            }
        }

        CustomCheckBox {
            id: preserveCb
            text: "Preserve sound"
            font.pixelSize: 9
            spacing: 6
        }
    }

    // ============================================================
    // Dialogs
    // ============================================================
    Dialog {
        id: renameDialog
        title: "Rename player"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        x: root.width / 2 - width / 2
        y: root.height / 2 - height / 2

        background: Rectangle {
            color: Theme.inputBackground
            border.color: Theme.borderHover
            border.width: 1
            radius: Theme.cardRadius
        }

        header: Label {
            text: "Rename player"
            color: Theme.textPrimary
            font.pixelSize: Theme.fontSizeHeading
            font.weight: Font.Bold
            padding: 12
        }

        contentItem: TextField {
            id: renameField
            text: slotName || ""
            color: Theme.textPrimary
            font.pixelSize: Theme.fontSizeNormal
            selectByMouse: true
            background: Rectangle {
                color: Theme.inputBackground
                border.color: renameField.activeFocus ? Theme.accentPurple : Theme.borderHover
                border.width: 1
                radius: Theme.borderRadius
                Behavior on border.color { ColorAnimation { duration: Theme.animDuration } }
            }
        }

        onAccepted: {
            if (renameField.text.trim() !== "") {
                Backend.soundboard.renamePlayer(slotId, renameField.text.trim())
            }
        }
    }

    Dialog {
        id: makePermanentDialog
        title: "Make file permanent"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        x: root.width / 2 - width / 2
        y: root.height / 2 - height / 2

        background: Rectangle {
            color: Theme.inputBackground
            border.color: Theme.borderHover
            border.width: 1
            radius: Theme.cardRadius
        }

        header: Label {
            text: "Enter permanent file name:"
            color: Theme.textPrimary
            font.pixelSize: Theme.fontSizeNormal
            font.weight: Font.Bold
            padding: 12
        }

        contentItem: TextField {
            id: permanentField
            text: {
                var parts = (filePath || "").toString().split("/")
                return parts[parts.length - 1] || ""
            }
            color: Theme.textPrimary
            font.pixelSize: Theme.fontSizeNormal
            selectByMouse: true
            background: Rectangle {
                color: Theme.appBackground
                border.color: permanentField.activeFocus ? Theme.accentPurple : Theme.borderHover
                border.width: 1
                radius: Theme.borderRadius
                Behavior on border.color { ColorAnimation { duration: Theme.animDuration } }
            }
        }

        onAccepted: {
            if (permanentField.text.trim() !== "") {
                Backend.actions.dispatchMakePermanent(slotId, permanentField.text.trim())
            }
        }
    }

    MessageDialog {
        id: removeConfirmDialog
        title: "Confirm delete"
        text: "Are you sure you want to delete this player?"
        informativeText: (filePath && filePath !== "") || (playHotkey && playHotkey !== "") || (assignHotkey && assignHotkey !== "") ?
            "It has an assigned audio file and/or hotkey bindings." : ""
        buttons: MessageDialog.Yes | MessageDialog.No
        onAccepted: Backend.soundboard.removePlayer(slotId)
    }

    FileDialog {
        id: assignFileDialog
        title: "Select audio file"
        nameFilters: ["Audio files (*.wav *.mp3 *.ogg)"]
        onAccepted: {
            if (selectedFile) Backend.soundboard.assignAudioFile(slotId, selectedFile)
        }
    }

    // ============================================================
    // Backend connections
    // ============================================================
    Connections {
        target: Backend.soundboard
        function onPlayerPositionChanged(id, position) {
            if (id === slotId) waveformView.playPositionMs = position
        }
        function onWaveformGenerated(playerId, data) {
            if (playerId === slotId && data.isValid) {
                waveformView.waveformData = data
                waveformView.startMs = startTimeMs
                waveformView.endMs = endTimeMs
            }
        }
    }

    Component.onCompleted: {
        if (filePath && filePath !== "") {
            var wfData = Backend.soundboard.getWaveformData(slotId)
            if (wfData && wfData.isValid) {
                waveformView.waveformData = wfData
            } else {
                Backend.soundboard.loadWaveformData(slotId, filePath)
            }
        }
    }

    function openHotkeyDialog() {
        Utils.openDialog("HotkeyDialog.qml", {
            playerId: slotId,
            playKey: playHotkey || "",
            assignKey: assignHotkey || ""
        }, function(win) {
            Backend.soundboard.setHotkeys(slotId, win.playKey, win.assignKey)
            win.close()
        })
    }
}
