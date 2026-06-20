import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Templates 2.15 as T
import QtQuick.Dialogs
import Saiko 1.0

Rectangle {
    id: root
    width: 210
    height: cardColumn.implicitHeight + 28
    color: cardHover.hovered ? "#141414" : "#111111"
    radius: 12
    border.color: cardHover.hovered ? "#262626" : "#1c1c1c"
    border.width: 1

    Behavior on color { ColorAnimation { duration: 180 } }
    Behavior on border.color { ColorAnimation { duration: 180 } }

    HoverHandler {
        id: cardHover
    }

    property var slotModel: null
    property int slotIndex: -1

    // ============================================================
    // Reusable dropdown menu, built on Templates to dodge the
    // Controls.Menu "highlighted" scoping bug seen with nested
    // component blocks.
    // ============================================================
    component CardMenu: T.Menu {
        id: menu
        width: 168
        margins: 0
        implicitHeight: contentItem.contentHeight + 10

        contentItem: ListView {
            implicitHeight: contentHeight
            model: menu.contentModel
            interactive: false
            spacing: 2
        }

        background: Rectangle {
            implicitWidth: 168
            color: "#171717"
            border.color: "#262626"
            border.width: 1
            radius: 8
        }

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 110 }
        }
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 80 }
        }
    }

    component CardMenuItem: T.MenuItem {
        id: menuItem
        implicitWidth: 160
        implicitHeight: 30
        padding: 4

        contentItem: Text {
            text: menuItem.text
            color: menuItem.hovered ? "white" : "#aaa"
            font.pixelSize: 11
            leftPadding: 8
            verticalAlignment: Text.AlignVCenter
            Behavior on color { ColorAnimation { duration: 100 } }
        }

        background: Rectangle {
            color: menuItem.hovered ? "#242424" : "transparent"
            radius: 6
            Behavior on color { ColorAnimation { duration: 100 } }
        }
    }

    // ============================================================
    // Reusable small action button (Play / Preview / Stop / Assign)
    // ============================================================
    component ActionButton: Rectangle {
        id: btn
        property string label: ""
        property color tint: "#888888"
        property bool filled: false
        signal clicked()

        Layout.fillWidth: true
        height: 26
        radius: 6
        color: filled
            ? Qt.rgba(tint.r, tint.g, tint.b, mouse.containsMouse ? 0.22 : 0.13)
            : (mouse.containsMouse ? "#1c1c1c" : "transparent")
        border.color: filled
            ? Qt.rgba(tint.r, tint.g, tint.b, mouse.containsMouse ? 0.7 : 0.4)
            : (mouse.containsMouse ? "#2a2a2a" : "#1f1f1f")
        border.width: 1
        Behavior on color { ColorAnimation { duration: 130 } }
        Behavior on border.color { ColorAnimation { duration: 130 } }

        Text {
            anchors.centerIn: parent
            text: btn.label
            color: filled ? btn.tint : (mouse.containsMouse ? "#ddd" : "#888")
            font.pixelSize: 10
            font.weight: Font.Medium
            Behavior on color { ColorAnimation { duration: 130 } }
        }

        MouseArea {
            id: mouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: btn.clicked()
        }
    }

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
                color: "white"
                font.pixelSize: 13
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
                Behavior on color { ColorAnimation { duration: 120 } }
                Behavior on border.color { ColorAnimation { duration: 120 } }

                Text {
                    anchors.centerIn: parent
                    text: "\u22EF"
                    color: optionsMouse.containsMouse ? "white" : "#666"
                    font.pixelSize: 15
                    Behavior on color { ColorAnimation { duration: 120 } }
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
            color: isTemporary ? "#d99a3d" : "#555555"
            font.pixelSize: 10
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
            color: "#0c0c0c"
            radius: 8
            border.color: "#1c1c1c"
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

        // --- Clip range panel ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: clipColumn.implicitHeight + 16
            color: "#151515"
            radius: 8
            border.color: "#1f1f1f"
            border.width: 1

            ColumnLayout {
                id: clipColumn
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                Text {
                    text: "SELECTED CLIP"
                    color: "#454545"
                    font.pixelSize: 8
                    font.letterSpacing: 1.2
                    font.weight: Font.Bold
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text { text: "Start"; color: "#777"; font.pixelSize: 9; Layout.preferredWidth: 30 }

                    SpinBox {
                        id: startSpin
                        Layout.fillWidth: true
                        implicitHeight: 24
                        from: 0
                        to: Math.max(1, durationSec * 1000)
                        stepSize: 100
                        value: startTimeMs
                        editable: true
                        font.pixelSize: 9

                        Binding {
                            target: startSpin
                            property: "value"
                            value: startTimeMs
                        }

                        contentItem: TextInput {
                            text: startSpin.textFromValue(startSpin.value, startSpin.locale)
                            color: "white"
                            font: startSpin.font
                            readOnly: !startSpin.editable
                            validator: startSpin.validator
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            verticalAlignment: TextInput.AlignVCenter
                            horizontalAlignment: TextInput.AlignHCenter
                            selectByMouse: true
                        }

                        background: Rectangle {
                            implicitHeight: 24
                            color: "#0e0e0e"
                            radius: 5
                            border.width: 1
                            border.color: startSpin.activeFocus ? "#BB86FC" : (startSpin.hovered ? "#2a2a2a" : "#1c1c1c")
                            Behavior on border.color { ColorAnimation { duration: 100 } }
                        }

                        onValueChanged: {
                            if (activeFocus && slotModel && slotIndex >= 0) {
                                var endVal = endSpin.value
                                if (value > endVal - 50) value = Math.max(from, endVal - 50)
                                slotModel.setClipRange(slotIndex, value, endSpin.value)
                            }
                        }
                        textFromValue: function(val) { return (val / 1000).toFixed(1) + "s" }
                        valueFromText: function(text) {
                            var v = parseFloat(text)
                            return isNaN(v) ? value : Math.round(v * 1000)
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text { text: "End"; color: "#777"; font.pixelSize: 9; Layout.preferredWidth: 30 }

                    SpinBox {
                        id: endSpin
                        Layout.fillWidth: true
                        implicitHeight: 24
                        from: 0
                        to: Math.max(1, durationSec * 1000)
                        stepSize: 100
                        value: endTimeMs === -1 ? Math.max(1, durationSec * 1000) : endTimeMs
                        editable: true
                        font.pixelSize: 9

                        Binding {
                            target: endSpin
                            property: "value"
                            value: endTimeMs === -1 ? Math.max(1, durationSec * 1000) : endTimeMs
                        }

                        contentItem: TextInput {
                            text: endSpin.textFromValue(endSpin.value, endSpin.locale)
                            color: "white"
                            font: endSpin.font
                            readOnly: !endSpin.editable
                            validator: endSpin.validator
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            verticalAlignment: TextInput.AlignVCenter
                            horizontalAlignment: TextInput.AlignHCenter
                            selectByMouse: true
                        }

                        background: Rectangle {
                            implicitHeight: 24
                            color: "#0e0e0e"
                            radius: 5
                            border.width: 1
                            border.color: endSpin.activeFocus ? "#BB86FC" : (endSpin.hovered ? "#2a2a2a" : "#1c1c1c")
                            Behavior on border.color { ColorAnimation { duration: 100 } }
                        }

                        onValueChanged: {
                            if (activeFocus && slotModel && slotIndex >= 0) {
                                var startVal = startSpin.value
                                if (value < startVal + 50) value = startVal + 50
                                slotModel.setClipRange(slotIndex, startSpin.value, value)
                            }
                        }
                        textFromValue: function(val) { return (val / 1000).toFixed(1) + "s" }
                        valueFromText: function(text) {
                            var v = parseFloat(text)
                            return isNaN(v) ? value : Math.round(v * 1000)
                        }
                    }
                }
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
                    color: "#222"

                    Rectangle {
                        x: 0
                        y: parent.height * volSlider.visualPosition
                        width: parent.width
                        height: parent.height * (1.0 - volSlider.visualPosition)
                        radius: 2
                        color: "#BB86FC"
                    }
                }

                handle: Rectangle {
                    x: volSlider.leftPadding + volSlider.availableWidth / 2 - width / 2
                    y: volSlider.topPadding + volSlider.visualPosition * volSlider.availableHeight - height / 2
                    width: 14
                    height: 14
                    radius: 7
                    color: volSlider.pressed ? "#BB86FC" : (volSlider.hovered ? "#fff" : "#888")
                    border.color: volSlider.hovered ? "#BB86FC" : "#555"
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

                    ActionButton {
                        label: "Play"
                        tint: "#4caf50"
                        filled: true
                        onClicked: Backend.soundboard.playPlayer(slotId)
                    }
                    ActionButton {
                        label: "Preview"
                        tint: "#03DAC6"
                        filled: true
                        onClicked: Backend.soundboard.playPlayerPreview(slotId)
                    }
                }

                ActionButton {
                    label: "Stop"
                    tint: "#e35d5d"
                    filled: true
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
                color: "#777"
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
            color: "#1e1e1e"
        }

        // --- Footer: assign / remove ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Rectangle {
                id: assignBtn
                Layout.fillWidth: true
                height: 28
                radius: 7
                color: assignMouse.containsMouse ? "#1c1c1c" : "transparent"
                border.color: assignMouse.containsMouse ? "#2a2a2a" : "#1c1c1c"
                border.width: 1
                Behavior on color { ColorAnimation { duration: 130 } }
                Behavior on border.color { ColorAnimation { duration: 130 } }

                Text {
                    anchors.centerIn: parent
                    text: "Assign"
                    color: assignMouse.containsMouse ? "white" : "#999"
                    font.pixelSize: 10
                    Behavior on color { ColorAnimation { duration: 130 } }
                }

                MouseArea {
                    id: assignMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: assignMenu.open()
                }

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

            Rectangle {
                id: removeBtn
                width: 28
                height: 28
                radius: 7
                color: removeMouse.containsMouse ? "#2c1212" : "transparent"
                border.color: removeMouse.containsMouse ? "#a13c3c" : "#1c1c1c"
                border.width: 1
                Behavior on color { ColorAnimation { duration: 130 } }
                Behavior on border.color { ColorAnimation { duration: 130 } }

                Text {
                    anchors.centerIn: parent
                    text: "\u2715"
                    color: removeMouse.containsMouse ? "#ff6b6b" : "#666"
                    font.pixelSize: 11
                    Behavior on color { ColorAnimation { duration: 130 } }
                }

                MouseArea {
                    id: removeMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: removeConfirmDialog.open()
                }
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
            color: "#161616"
            border.color: "#222"
            border.width: 1
            radius: 8
        }

        header: Label {
            text: "Rename player"
            color: "white"
            font.pixelSize: 13
            font.weight: Font.Bold
            padding: 12
        }

        contentItem: TextField {
            id: renameField
            text: slotName || ""
            color: "white"
            font.pixelSize: 12
            selectByMouse: true
            background: Rectangle {
                color: "#121212"
                border.color: renameField.activeFocus ? "#BB86FC" : "#222"
                border.width: 1
                radius: 6
                Behavior on border.color { ColorAnimation { duration: 150 } }
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
            color: "#161616"
            border.color: "#222"
            border.width: 1
            radius: 8
        }

        header: Label {
            text: "Enter permanent file name:"
            color: "white"
            font.pixelSize: 12
            font.weight: Font.Bold
            padding: 12
        }

        contentItem: TextField {
            id: permanentField
            text: {
                var parts = (filePath || "").toString().split("/")
                return parts[parts.length - 1] || ""
            }
            color: "white"
            font.pixelSize: 12
            selectByMouse: true
            background: Rectangle {
                color: "#121212"
                border.color: permanentField.activeFocus ? "#BB86FC" : "#222"
                border.width: 1
                radius: 6
                Behavior on border.color { ColorAnimation { duration: 150 } }
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
        var component = Qt.createComponent("qrc:/qml/HotkeyDialog.qml")
        if (component.status === Component.Ready) {
            var win = component.createObject(null, {
                playerId: slotId,
                playKey: playHotkey || "",
                assignKey: assignHotkey || ""
            })
            win.accepted.connect(function() {
                Backend.soundboard.setHotkeys(slotId, win.playKey, win.assignKey)
                win.close()
            })
            win.rejected.connect(function() { win.close() })
            win.show()
        } else {
            if (component.status === Component.Error)
                console.error("HotkeyDialog error:", component.errorString())
        }
    }
}
