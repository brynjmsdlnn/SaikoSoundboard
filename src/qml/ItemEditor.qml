import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Saiko 1.0
import "utils.js" as Utils

Rectangle {
    id: editor

    // ── Appearance ──────────────────────────────────────────────────────────
    color: Theme.cardBackground
    border.color: Theme.borderDefault
    border.width: 1
    implicitWidth: 420

    // ── Public properties ────────────────────────────────────────────────────
    property var slotModel
    property int slotIndex: -1
    property string slotId: ""
    property string slotName: ""
    property string filePath: ""
    property bool isTemporary: false
    property int startTimeMs: 0
    property int endTimeMs: 0
    property real durationSec: 0.0
    property real volume: 1.0
    property int outputRouting: 0
    property string playHotkey: ""
    property string assignHotkey: ""
    property var waveformData: null

    // ── Helpers ───────────────────────────────────────────────────────────────
    readonly property bool hasSlot: slotIndex >= 0 && slotId !== ""
    // Safely evaluate to a boolean to prevent "Unable to assign [undefined] to bool"
    readonly property bool isTemp: !!(editor && editor.isTemporary)

    // ── Empty state ───────────────────────────────────────────────────────────
    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width - 64
        visible: !editor.hasSlot
        spacing: 16

        Image {
            source: "image://icons/sliders-horizontal?color=%23b0b0b0"
            width: 48
            height: 48
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            text: "No Slot Selected"
            color: Theme.textPrimary
            font.pixelSize: 16
            font.weight: Font.Bold
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            text: "Select a soundboard slot from the grid to view and edit its details."
            color: Theme.textDim
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }
    }

    // ── Main content ──────────────────────────────────────────────────────────
    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        visible: editor.hasSlot
        topPadding: 20
        bottomPadding: 20
        leftPadding: 20
        rightPadding: 20
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
            width: scroll.availableWidth
            spacing: 20

            // Title
            Text {
                text: "Slot Details"
                color: Theme.textPrimary
                font.pixelSize: 18
                font.weight: Font.Bold
            }

            // ── Slot name ──────────────────────────────────────────────────
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                SectionLabel {
                    text: "SLOT NAME"
                }

                TextField {
                    id: nameField
                    Layout.fillWidth: true
                    implicitHeight: 36
                    text: editor.slotName
                    placeholderText: "Unnamed Slot"
                    color: Theme.textPrimary
                    font.pixelSize: 14
                    selectByMouse: true
                    rightPadding: 32 // Prevent text from overlapping the icons

                    // Custom properties for state management
                    property bool showSuccess: false
                    // isDirty is true if focused AND the text has been modified from the saved state
                    property bool isDirty: activeFocus && text !== editor.slotName

                    SequentialAnimation {
                        id: successAnim
                        PropertyAction {
                            target: nameField
                            property: "showSuccess"
                            value: true
                        }
                        PauseAnimation {
                            duration: 1500
                        }
                        PropertyAction {
                            target: nameField
                            property: "showSuccess"
                            value: false
                        }
                    }

                    background: Rectangle {
                        color: Theme.inputBackground
                        border.color: nameField.showSuccess ? Theme.accentGreen : (nameField.activeFocus ? Theme.accentPurple : Theme.borderDefault)
                        radius: 6

                        Behavior on border.color {
                            ColorAnimation {
                                duration: 200
                            }
                        }
                    }

                    // 1. "Press Enter" Hint Icon (Shows only when there are unsaved edits)
                    Image {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.rightMargin: 10
                        // 'corner-down-left' is the standard Feather/Lucide icon for the Enter/Return key
                        source: "image://icons/corner-down-left?color=%23b0b0b0"
                        sourceSize: Qt.size(16, 16)
                        opacity: nameField.isDirty && !nameField.showSuccess ? 1.0 : 0.0

                        Behavior on opacity {
                            NumberAnimation {
                                duration: 200
                            }
                        }
                    }

                    // 2. Success Checkmark Icon
                    Image {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.rightMargin: 10
                        source: "image://icons/check?color=%234caf50"
                        sourceSize: Qt.size(16, 16)
                        opacity: nameField.showSuccess ? 1.0 : 0.0

                        Behavior on opacity {
                            NumberAnimation {
                                duration: 200
                            }
                        }
                    }

                    onEditingFinished: {
                        // Only save and animate if there's an actual change
                        if (editor.slotId && text.trim() !== editor.slotName) {
                            Backend.soundboard.renamePlayer(editor.slotId, text.trim());
                            successAnim.restart();
                        }
                    }
                }
            }

            // ── Audio source ───────────────────────────────────────────────
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                SectionLabel {
                    text: "AUDIO SOURCE"
                }

                Rectangle {
                    id: sourceBox
                    Layout.fillWidth: true
                    implicitHeight: 36
                    radius: 6
                    color: assignBtnArea.containsMouse ? Theme.recessedBackground : Theme.inputBackground

                    border.color: isTemp ? (assignBtnArea.containsMouse ? "#d99a3d" : "#b8860b") : (assignBtnArea.containsMouse ? Theme.accentPurple : Theme.borderDefault)
                    border.width: 1

                    Behavior on border.color {
                        ColorAnimation {
                            duration: 150
                        }
                    }
                    Behavior on color {
                        ColorAnimation {
                            duration: 150
                        }
                    }

                    // 1. MAIN MOUSE AREA
                    MouseArea {
                        id: assignBtnArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: assignMenu.open()
                    }

                    // 2. VISUAL CONTENT
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 8
                        spacing: 8

                        Image {
                            source: "image://icons/music?color=%23888888"
                            smooth: true
                            sourceSize: Qt.size(16, 16)
                        }

                        // TEMP Badge
                        Rectangle {
                            visible: isTemp
                            implicitWidth: tempText.implicitWidth + 12
                            implicitHeight: 20
                            radius: 4
                            color: "#33250a" // Dark recessed amber
                            border.color: "#b8860b" // Dimmer amber border
                            border.width: 1

                            Text {
                                id: tempText
                                anchors.centerIn: parent
                                text: "TEMP"
                                color: "#d99a3d" // Bright amber text
                                font.pixelSize: 10
                                font.bold: true
                            }
                        }

                        // File Name Text
                        Text {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                            text: (editor && editor.filePath) ? editor.filePath.split("/").pop() : "No file assigned"
                            color: (editor && editor.filePath) ? Theme.textPrimary : Theme.textSecondary
                            font.pixelSize: 13
                            font.italic: !(editor && editor.filePath)
                        }

                        // Dropdown Indicator (Chevron fixed to explicitly use gray so it's visible)
                        Image {
                            source: "image://icons/chevron-down?color=%23888888"
                            sourceSize: Qt.size(16, 16)
                        }

                        // Make Permanent Button
                        Rectangle {
                            visible: isTemp
                            implicitWidth: 26
                            implicitHeight: 26
                            radius: 4
                            color: permanentBtnArea.containsMouse ? "#33250a" : "#1f1606"
                            border.color: permanentBtnArea.containsMouse ? "#d99a3d" : "transparent"
                            border.width: 1

                            Image {
                                anchors.centerIn: parent
                                source: "image://icons/save?color=%23d99a3d"
                                sourceSize: Qt.size(14, 14)
                            }

                            ToolTip {
                                visible: permanentBtnArea.containsMouse
                                text: "Save to disk (Make permanent)"
                                delay: 400
                            }

                            MouseArea {
                                id: permanentBtnArea
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                hoverEnabled: true
                                // Open the rename dialog instead of instantly saving
                                onClicked: saveFileNameDialog.open()
                            }
                        }
                    }

                    // 3. DROPDOWN MENU
                    CardMenu {
                        id: assignMenu
                        y: parent.height + 4
                        // Align the right side of the menu to the right side of the box
                        x: sourceBox.width - width

                        CardMenuItem {
                            text: "From file..."
                            onClicked: assignFileDialog.open()
                        }
                        CardMenuItem {
                            text: "From replay buffer"
                            onClicked: {
                                if (editor && editor.slotId)
                                    Backend.actions.dispatchAssignReplay(editor.slotId, preserveCb.checked);
                            }
                        }
                    }
                }

                CustomCheckBox {
                    id: preserveCb
                    text: "Preserve replay buffer on assign"
                    font.pixelSize: 11
                    checked: true
                }

                // 4. SAVE FILE NAME DIALOG
                Dialog {
                    id: saveFileNameDialog
                    parent: Overlay.overlay
                    x: Math.round((parent.width - width) / 2)
                    y: Math.round((parent.height - height) / 2)
                    width: 300
                    modal: true
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                    background: Rectangle {
                        color: Theme.inputBackground
                        border.color: Theme.borderDefault
                        radius: 8
                    }

                    contentItem: ColumnLayout {
                        spacing: 12

                        Text {
                            text: "Save Permanent File"
                            font.bold: true
                            font.pixelSize: 14
                            color: Theme.textPrimary
                        }

                        Text {
                            text: "Enter a file name:"
                            color: Theme.textSecondary
                            font.pixelSize: 12
                        }

                        TextField {
                            id: saveNameField
                            Layout.fillWidth: true
                            text: saveFileNameDialog.stripExtension(editor.filePath ? editor.filePath.split("/").pop() : editor.slotName)
                            selectByMouse: true
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            background: Rectangle {
                                color: Theme.recessedBackground
                                border.color: saveNameField.activeFocus ? Theme.accentPurple : Theme.borderDefault
                                radius: 6
                            }

                            onAccepted: saveFileNameDialog.saveAction()
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight
                            spacing: 8
                            Layout.topMargin: 8

                            ThemedButton {
                                text: "Cancel"
                                small: true
                                onClicked: saveFileNameDialog.close()
                            }

                            ThemedButton {
                                text: "Save"
                                small: true
                                onClicked: saveFileNameDialog.saveAction()
                            }
                        }
                    }

                    function stripExtension(name) {
                        var idx = name.lastIndexOf(".");
                        return idx > 0 ? name.substring(0, idx) : name;
                    }

                    function saveAction() {
                        if (editor && editor.slotId) {
                            var finalName = saveNameField.text.trim() || editor.slotName;
                            Backend.actions.dispatchMakePermanent(editor.slotId, finalName);
                        }
                        saveFileNameDialog.close();
                    }

                    onOpened: {
                        var raw = editor.filePath ? editor.filePath.split("/").pop() : editor.slotName;
                        saveNameField.text = stripExtension(raw);
                        saveNameField.forceActiveFocus();
                        saveNameField.selectAll();
                    }
                }
            }

            // ── Trim & waveform ────────────────────────────────────────────
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8
                visible: editor.filePath !== ""

                SectionLabel {
                    text: "TRIM & TIMING"
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 72
                    color: Theme.recessedBackground
                    radius: 6
                    border.color: Theme.borderDefault
                    clip: true

                    WaveformView {
                        id: wf
                        anchors {
                            fill: parent
                            margins: 6
                        }
                        startMs: editor.startTimeMs
                        endMs: editor.endTimeMs
                        waveformData: editor.waveformData

                        onTrimRangeChanged: (s, e) => {
                            if (editor.slotIndex >= 0)
                                editor.slotModel.setClipRange(editor.slotIndex, s, e);
                        }
                        onTrimRangeCommit: (s, e) => {
                            if (editor.slotIndex >= 0)
                                editor.slotModel.setClipRange(editor.slotIndex, s, e);
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 24

                // ── LEFT COLUMN: Volume (Vertical) ─────────────────────────────
                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.minimumWidth: 60
                    spacing: 8

                    SectionLabel {
                        text: "VOLUME"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Slider {
                        id: volSlider
                        orientation: Qt.Vertical
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter
                        from: 0
                        to: 100
                        value: editor.volume * 100
                        live: true

                        background: Rectangle {
                            x: volSlider.leftPadding + volSlider.availableWidth / 2 - width / 2
                            y: volSlider.topPadding
                            width: 4
                            height: volSlider.availableHeight
                            radius: 2
                            color: Theme.borderDefault

                            // The filled portion of the vertical slider
                            Rectangle {
                                y: volSlider.visualPosition * parent.height
                                width: parent.width
                                height: parent.height - y
                                color: Theme.accentPurple
                                radius: 2
                            }
                        }

                        handle: Rectangle {
                            x: volSlider.leftPadding + volSlider.availableWidth / 2 - width / 2
                            y: volSlider.topPadding + volSlider.visualPosition * volSlider.availableHeight - height / 2
                            width: 14
                            height: 14
                            radius: 7
                            color: volSlider.pressed ? Theme.accentPurple : Theme.textPrimary
                            border.color: Theme.accentPurple
                            border.width: 1
                        }

                        onMoved: {
                            if (editor.slotIndex >= 0)
                                editor.slotModel.setVolume(editor.slotIndex, value / 100);
                        }
                    }

                    Text {
                        text: (editor.volume * 100).toFixed(0) + "%"
                        color: Theme.textPrimary
                        font.pixelSize: 12
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                }

                // ── RIGHT COLUMN: Routing, Hotkeys & Actions ───────────────────
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 16

                    // ── Output routing ─────────────────────────────────────────────
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        SectionLabel {
                            text: "OUTPUT ROUTING"
                        }

                        CustomComboBox {
                            Layout.fillWidth: true
                            implicitHeight: 34
                            model: [
                                {
                                    text: "Broadcast & monitor",
                                    value: 0
                                },
                                {
                                    text: "Broadcast only",
                                    value: 1
                                },
                                {
                                    text: "Monitor only",
                                    value: 2
                                }
                            ]
                            textRole: "text"
                            valueRole: "value"
                            currentIndex: editor.outputRouting
                            onCurrentValueChanged: {
                                if (editor.slotIndex >= 0)
                                    editor.slotModel.setRouting(editor.slotIndex, currentValue);
                            }
                        }
                    }

                    // ── Hotkeys ────────────────────────────────────────────────────
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        SectionLabel {
                            text: "HOTKEYS"
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 48
                            color: Theme.inputBackground
                            border.color: Theme.borderDefault
                            radius: 6

                            // 1. Timer to toggle between states every 3 seconds
                            Timer {
                                id: cycleTimer
                                interval: 3000
                                running: true
                                repeat: true
                                onTriggered: hotkeyContainer.state = (hotkeyContainer.state === "showAssign") ? "showPlay" : "showAssign"
                            }

                            RowLayout {
                                anchors {
                                    fill: parent
                                    leftMargin: 12
                                    rightMargin: 8
                                }
                                spacing: 8

                                Image {
                                    source: "image://icons/keyboard?color=%23b0b0b0"
                                    smooth: true
                                    sourceSize: Qt.size(18, 18)
                                }

                                // 2. The Carousel Container
                                Item {
                                    id: hotkeyContainer
                                    Layout.fillWidth: true
                                    implicitHeight: 24
                                    clip: true // Keeps things clean if you decide to use slide animations later

                                    state: "showPlay"

                                    // --- PLAY SHORTCUT ---
                                    RowLayout {
                                        id: playRow
                                        anchors.verticalCenter: parent.verticalCenter
                                        spacing: 6
                                        opacity: 1.0 // Default state

                                        Text {
                                            text: "Play Action:"
                                            font.pixelSize: 12;
                                            color: Theme.textSecondary
                                        }

                                        // Inline badge style
                                        Rectangle {
                                            implicitWidth: playText.implicitWidth + 12
                                            implicitHeight: 20
                                            color: "#2a2a2a"
                                            border.color: "#3a3a3a"
                                            radius: 4
                                            Text {
                                                id: playText
                                                anchors.centerIn: parent
                                                text: editor.playHotkey || "—"
                                                font.pixelSize: 11
                                                font.weight: Font.DemiBold
                                                color: "#e0e0e0"
                                            }
                                        }
                                    }

                                    // --- ASSIGN SHORTCUT ---
                                    RowLayout {
                                        id: assignRow
                                        anchors.verticalCenter: parent.verticalCenter
                                        spacing: 6
                                        opacity: 0.0 // Hidden by default

                                        Text {
                                            text: "Assign from Replay Action:"
                                            font.pixelSize: 12;
                                            color: Theme.textSecondary
                                        }

                                        Rectangle {
                                            implicitWidth: assignText.implicitWidth + 12
                                            implicitHeight: 20
                                            color: "#2a2a2a"
                                            border.color: "#3a3a3a"
                                            radius: 4
                                            Text {
                                                id: assignText
                                                anchors.centerIn: parent
                                                text: editor.assignHotkey || "—"
                                                font.pixelSize: 11
                                                font.weight: Font.DemiBold
                                                color: "#e0e0e0"
                                            }
                                        }
                                    }

                                    // 3. Smooth Fade Transitions
                                    states: [
                                        State {
                                            name: "showPlay"
                                            PropertyChanges { target: playRow; opacity: 1.0 }
                                            PropertyChanges { target: assignRow; opacity: 0.0 }
                                        },
                                        State {
                                            name: "showAssign"
                                            PropertyChanges { target: playRow; opacity: 0.0 }
                                            PropertyChanges { target: assignRow; opacity: 1.0 }
                                        }
                                    ]

                                    transitions: Transition {
                                        NumberAnimation { property: "opacity"; duration: 250; easing.type: Easing.InOutQuad }
                                    }
                                }

                                ThemedButton {
                                    text: "Rebind"
                                    small: true
                                    implicitWidth: 64
                                    implicitHeight: 28
                                    onClicked: openHotkeyDialog()
                                }
                            }
                        }
                    }

                    // ── Action buttons ─────────────────────────────────────────────
                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 4 // Give a tiny bit of breathing room from the hotkeys above
                        spacing: 16

                        Repeater {
                            model: [
                                {
                                    label: "Play",
                                    icon: "play",
                                    color: "4caf50",
                                    accentProp: "accentGreen",
                                    hoverBg: "",
                                    action: () => {
                                        if (editor.slotId)
                                            Backend.soundboard.playPlayer(editor.slotId);
                                    }
                                },
                                {
                                    label: "Preview",
                                    icon: "headphones",
                                    color: "03DAC6",
                                    accentProp: "accentTeal",
                                    hoverBg: "",
                                    action: () => {
                                        if (editor.slotId)
                                            Backend.soundboard.playPlayerPreview(editor.slotId);
                                    }
                                },
                                {
                                    label: "Stop",
                                    icon: "square",
                                    color: "e35d5d",
                                    accentProp: "accentRed",
                                    hoverBg: "",
                                    action: () => {
                                        if (editor.slotId)
                                            Backend.soundboard.stopPlayer(editor.slotId);
                                    }
                                },
                                {
                                    label: "Delete",
                                    icon: "trash-2",
                                    color: "888888",
                                    accentProp: "accentRed",
                                    hoverBg: "#2a1a1a",
                                    action: () => removeConfirmDialog.open()
                                }
                            ]

                            delegate: Button {
                                implicitWidth: 80
                                implicitHeight: 76

                                // Expose hover state cleanly
                                property bool isDelete: modelData.label === "Delete"

                                background: Rectangle {
                                    color: parent.hovered ? (isDelete ? modelData.hoverBg : Theme.inputBackground) : Theme.recessedBackground
                                    radius: 8
                                    border.color: parent.hovered ? Theme[modelData.accentProp] : Theme.borderDefault
                                    border.width: 1
                                }

                                contentItem: ColumnLayout {
                                    spacing: 6

                                    Image {
                                        source: "image://icons/" + modelData.icon + "?color=%23" + (isDelete && parent.parent.hovered ? "e35d5d" : modelData.color)
                                        smooth: true
                                        sourceSize: Qt.size(24, 24)
                                        Layout.alignment: Qt.AlignHCenter
                                    }
                                    Text {
                                        text: modelData.label
                                        color: isDelete && parent.parent.hovered ? Theme.accentRed : Theme.textPrimary
                                        font.pixelSize: 12
                                        font.weight: Font.Medium
                                        Layout.alignment: Qt.AlignHCenter
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    acceptedButtons: Qt.NoButton
                                    hoverEnabled: true
                                }
                                onClicked: modelData.action()
                            }
                        }
                    }
                }
            }
        }
    }

    // ── Dialogs ───────────────────────────────────────────────────────────────
    FileDialog {
        id: assignFileDialog
        title: "Select audio file"
        nameFilters: ["Audio files (*.wav *.mp3 *.ogg)"]
        onAccepted: {
            if (selectedFile && editor.slotId)
                Backend.soundboard.assignAudioFile(editor.slotId, selectedFile);
        }
    }

    MessageDialog {
        id: removeConfirmDialog
        title: "Confirm delete"
        text: "Delete this player?"
        buttons: MessageDialog.Yes | MessageDialog.No
        onAccepted: {
            if (editor.slotId)
                Backend.soundboard.removePlayer(editor.slotId);
        }
    }

    // ── Connections ───────────────────────────────────────────────────────────
    Connections {
        target: SlotModel
        function onDataChanged(topLeft, bottomRight) {
            if (editor.slotIndex >= topLeft.row && editor.slotIndex <= bottomRight.row)
                updateProperties();
        }
        function onModelReset() {
            updateProperties();
        }
    }

    Connections {
        target: Backend.soundboard
        function onWaveformGenerated(playerId, data) {
            if (playerId === editor.slotId && data.isValid)
                editor.waveformData = data;
        }
        function onPlayerPositionChanged(playerId, pos) {
            if (playerId === editor.slotId)
                wf.playPositionMs = pos;
        }
    }

    // ── Signal handlers ───────────────────────────────────────────────────────
    onSlotIndexChanged: updateProperties()
    onFilePathChanged: loadWaveform()

    // ── Functions ─────────────────────────────────────────────────────────────
    function loadWaveform() {
        if (!editor.filePath) {
            editor.waveformData = null;
            return;
        }
        const cached = Backend.soundboard.getWaveformData(editor.slotId);
        if (cached?.isValid)
            editor.waveformData = cached;
        else
            Backend.soundboard.loadWaveformData(editor.slotId, editor.filePath);
    }

    function openHotkeyDialog() {
        Utils.openDialog("HotkeyDialog.qml", {
            playKey: editor.playHotkey || "",
            assignKey: editor.assignHotkey || ""
        }, function (win) {
            Backend.soundboard.setHotkeys(editor.slotId, win.playKey, win.assignKey);
            win.close();
        });
    }

    function updateProperties() {
        const empty = editor.slotIndex < 0 || editor.slotIndex >= SlotModel.rowCount();
        if (empty) {
            editor.slotId = "";
            editor.slotName = "";
            editor.filePath = "";
            editor.isTemporary = false;
            editor.startTimeMs = 0;
            editor.endTimeMs = 0;
            editor.durationSec = 0;
            editor.volume = 1;
            editor.outputRouting = 0;
            editor.playHotkey = "";
            editor.assignHotkey = "";
            return;
        }
        const d = SlotModel.get(editor.slotIndex);
        editor.slotId = d.slotId ?? "";
        editor.slotName = d.slotName ?? "";
        editor.filePath = d.filePath ?? "";
        editor.isTemporary = d.isTemporary ?? false;
        editor.startTimeMs = d.startTimeMs ?? 0;
        editor.endTimeMs = d.endTimeMs ?? 0;
        editor.durationSec = d.durationSec ?? 0;
        editor.volume = d.volume ?? 1;
        editor.outputRouting = d.outputRouting ?? 0;
        editor.playHotkey = d.playHotkey ?? "";
        editor.assignHotkey = d.assignHotkey ?? "";
        loadWaveform();
    }

    // ── Internal components ───────────────────────────────────────────────────

    // Reusable section label to avoid repeating the same Text styling four times
    component SectionLabel: Text {
        color: Theme.textDim
        font.pixelSize: 10
        font.bold: true
        font.letterSpacing: 0.5
    }

    // This is your custom "Key Cap" component defined inline
    component HotkeyBadge : Rectangle {
        property alias text: badgeText.text

        implicitWidth: badgeText.implicitWidth + 12  // Dynamic width based on key length (e.g., "Space" vs "F")
        implicitHeight: 20
        color: "#2a2a2a"                            // Slightly darker/different than input background
        border.color: "#3a3a3a"
        radius: 4

        Text {
            id: badgeText
            anchors.centerIn: parent
            font.pixelSize: 11
            font.weight: Font.DemiBold
            color: "#e0e0e0"
        }
    }
}
