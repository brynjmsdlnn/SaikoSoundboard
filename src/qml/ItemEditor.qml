import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Saiko 1.0
import "utils.js" as Utils

Rectangle {
    id: editor
    color: Theme.cardBackground
    border.color: Theme.borderDefault
    border.width: 1
    implicitWidth: 500

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

    // --- EMPTY STATE ---
    ColumnLayout {
        id: emptyState
        anchors.centerIn: parent
        width: parent.width - 64
        visible: editor.slotIndex < 0 || !editor.slotId
        spacing: 12
        opacity: 0.8

        Text {
            text: "🎛️"
            font.pixelSize: 42
            color: Theme.textDim
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            text: "No Slot Selected"
            color: Theme.textPrimary
            font.pixelSize: 18
            font.weight: Font.Bold
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            text: "Please select a soundboard slot from the grid to view and edit its details."
            color: Theme.textDim
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
        }
    }

    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        visible: !emptyState.visible
        topPadding: 16
        bottomPadding: 16
        leftPadding: 16
        rightPadding: 16

        ColumnLayout {
            width: scroll.availableWidth
            spacing: 10

            // Header
            Text {
                text: "Slot Details"
                color: Theme.textPrimary
                font.pixelSize: 18
                font.weight: Font.Bold
            }

            // SLOT NAME
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text { text: "SLOT NAME"; color: Theme.textDim; font.pixelSize: 11; font.bold: true }
                TextField {
                    id: nameField
                    Layout.fillWidth: true
                    implicitHeight: 30
                    text: editor.slotName
                    color: Theme.textPrimary
                    selectByMouse: true
                    background: Rectangle {
                        color: Theme.inputBackground
                        border.color: nameField.activeFocus ? Theme.accentPurple : Theme.borderDefault
                        radius: 6
                    }
                    onEditingFinished: if(editor.slotId) Backend.soundboard.renamePlayer(editor.slotId, text.trim())
                }
            }

            // AUDIO FILE
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text { text: "AUDIO FILE"; color: Theme.textDim; font.pixelSize: 11; font.bold: true }
                Text {
                    Layout.fillWidth: true
                    text: editor.filePath ? editor.filePath.split("/").pop() : "No file assigned"
                    color: editor.filePath ? Theme.textPrimary : Theme.textDim
                    elide: Text.ElideMiddle
                    font.pixelSize: 13
                }
                ThemedButton {
                    text: "Assign"
                    Layout.fillWidth: true
                    small: true
                    onClicked: assignMenu.open()
                    CardMenu {
                        id: assignMenu
                        y: parent.height + 4
                        CardMenuItem { text: "From file..."; onClicked: assignFileDialog.open() }
                        CardMenuItem { text: "From replay buffer"; onClicked: if(editor.slotId) Backend.actions.dispatchAssignReplay(editor.slotId, preserveCb.checked) }
                    }
                }
                CustomCheckBox {
                    id: preserveCb
                    text: "Preserve replay buffer on assign"
                    font.pixelSize: 11
                }
            }

            // WAVEFORM & TRIM
            ColumnLayout {
                Layout.fillWidth: true; spacing: 4; visible: editor.filePath !== ""
                Text { text: "WAVEFORM & TRIM"; color: Theme.textDim; font.pixelSize: Theme.fontSizeSmall; font.bold: true }
                Rectangle {
                    Layout.fillWidth: true; height: 64
                    color: Theme.recessedBackground; radius: Theme.cardRadius; border.color: Theme.borderDefault; border.width: 1; clip: true
                    WaveformView {
                        id: wf
                        anchors.fill: parent; anchors.margins: 4
                        startMs: editor.startTimeMs; endMs: editor.endTimeMs; waveformData: editor.waveformData
                        onTrimRangeChanged: (s,e) => { if(editor.slotIndex>=0) editor.slotModel.setClipRange(editor.slotIndex, s, e) }
                        onTrimRangeCommit: (s,e) => { if(editor.slotIndex>=0) editor.slotModel.setClipRange(editor.slotIndex, s, e) }
                    }
                }
                ClipRangeEditor {
                    Layout.fillWidth: true
                    startMs: editor.startTimeMs; endMs: editor.endTimeMs; durationSec: editor.durationSec
                    onClipRangeChanged: (s,e) => { if(editor.slotIndex>=0) editor.slotModel.setClipRange(editor.slotIndex, s, e) }
                }
            }

            // VOLUME - BALIK SA DATI MONG PURPLE
            ColumnLayout {
                Layout.fillWidth: true; spacing: 4
                Text { text: "VOLUME"; color: Theme.textDim; font.pixelSize: Theme.fontSizeSmall; font.bold: true }
                RowLayout {
                    Layout.fillWidth: true; spacing: 8
                    Slider {
                        id: volSlider
                        Layout.fillWidth: true
                        from: 0; to: 100; value: editor.volume * 100; live: true
                        background: Rectangle {
                            x: volSlider.leftPadding; y: volSlider.topPadding + volSlider.availableHeight/2 - height/2
                            width: volSlider.availableWidth; height: 4; radius: 2; color: Theme.borderDefault
                            Rectangle { width: volSlider.visualPosition * parent.width; height: parent.height; color: Theme.accentPurple; radius: 2 }
                        }
                        handle: Rectangle {
                            x: volSlider.leftPadding + volSlider.visualPosition * volSlider.availableWidth - width/2
                            y: volSlider.topPadding + volSlider.availableHeight/2 - height/2
                            width: 14; height: 14; radius: 7
                            color: volSlider.pressed ? Theme.accentPurple : (volSlider.hovered ? Theme.textPrimary : Theme.textDim)
                            border.color: volSlider.hovered ? Theme.accentPurple : "#555"; border.width: 1
                        }
                        onMoved: if(editor.slotIndex>=0) editor.slotModel.setVolume(editor.slotIndex, value/100)
                    }
                    Text { text: (editor.volume*100).toFixed(0)+"%"; color: Theme.textPrimary; font.pixelSize: Theme.fontSizeSmall; Layout.preferredWidth: 32 }
                }
            }

            // OUTPUT ROUTING
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text { text: "OUTPUT ROUTING"; color: Theme.textDim; font.pixelSize: 11; font.bold: true }
                CustomComboBox {
                    Layout.fillWidth: true
                    implicitHeight: 28
                    model: [{text:"Broadcast & monitor",value:0},{text:"Broadcast only",value:1},{text:"Monitor only",value:2}]
                    textRole: "text"; valueRole: "value"
                    currentIndex: editor.outputRouting
                    onCurrentValueChanged: if(editor.slotIndex>=0) editor.slotModel.setRouting(editor.slotIndex, currentValue)
                }
            }

            // HOTKEYS
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text { text: "HOTKEYS"; color: Theme.textDim; font.pixelSize: 11; font.bold: true }
                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        Layout.fillWidth: true
                        text: "Play: " + (editor.playHotkey||"—") + "   Assign: " + (editor.assignHotkey||"—")
                        color: Theme.textSecondary
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                    ThemedButton { text: "Rebind"; small: true; implicitWidth: 60; onClicked: openHotkeyDialog() }
                }
            }

            Item { Layout.fillHeight: true } // push buttons down

            // PLAY/STOP
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                ThemedButton {
                    text: "▶ Play"
                    Layout.fillWidth: true
                    filled: true
                    accentColor: Theme.accentGreen
                    onClicked: if(editor.slotId) Backend.soundboard.playPlayer(editor.slotId)
                }
                ThemedButton {
                    text: "■ Stop"
                    Layout.fillWidth: true
                    filled: true
                    accentColor: Theme.accentRed
                    onClicked: if(editor.slotId) Backend.soundboard.stopPlayer(editor.slotId)
                }
            }
            ThemedButton {
                text: "Delete Slot"
                Layout.fillWidth: true
                small: true
                onClicked: removeConfirmDialog.open()
            }
        }
    }

    FileDialog { id: assignFileDialog; title: "Select audio file"; nameFilters: ["Audio files (*.wav *.mp3 *.ogg)"]; onAccepted: if(selectedFile && editor.slotId) Backend.soundboard.assignAudioFile(editor.slotId, selectedFile) }
    MessageDialog { id: removeConfirmDialog; title: "Confirm delete"; text: "Delete this player?"; buttons: MessageDialog.Yes | MessageDialog.No; onAccepted: if(editor.slotId) Backend.soundboard.removePlayer(editor.slotId) }

    Connections {
        target: SlotModel
        function onDataChanged(t,b,r){ if(editor.slotIndex>=t.row && editor.slotIndex<=b.row) updateProperties() }
        function onModelReset(){ updateProperties() }
    }
    onSlotIndexChanged: updateProperties()
    Connections {
        target: Backend.soundboard
        function onWaveformGenerated(playerId, data) { if (playerId === editor.slotId && data.isValid) editor.waveformData = data }
        function onPlayerPositionChanged(playerId, pos) { if (playerId === editor.slotId) wf.playPositionMs = pos }
    }
    onFilePathChanged: loadWaveform()
    function loadWaveform(){ if(editor.filePath){ var w=Backend.soundboard.getWaveformData(editor.slotId); if(w&&w.isValid) editor.waveformData=w; else Backend.soundboard.loadWaveformData(editor.slotId, editor.filePath) } else editor.waveformData=null }
    function openHotkeyDialog(){ Utils.openDialog("HotkeyDialog.qml",{playerId:editor.slotId,playKey:editor.playHotkey||"",assignKey:editor.assignHotkey||""},function(win){Backend.soundboard.setHotkeys(editor.slotId,win.playKey,win.assignKey);win.close()}) }
    function updateProperties(){
        if(editor.slotIndex<0||editor.slotIndex>=SlotModel.rowCount()){ editor.slotId="";editor.slotName="";editor.filePath="";editor.volume=1;editor.outputRouting=0;editor.playHotkey="";editor.assignHotkey="";editor.startTimeMs=0;editor.endTimeMs=0;editor.durationSec=0;return }
        var d=SlotModel.get(editor.slotIndex); editor.slotId=d.slotId||""; editor.slotName=d.slotName||""; editor.filePath=d.filePath||""; editor.isTemporary=d.isTemporary||false; editor.startTimeMs=d.startTimeMs||0; editor.endTimeMs=d.endTimeMs||0; editor.durationSec=d.durationSec||0; editor.volume=d.volume!==undefined?d.volume:1; editor.outputRouting=d.outputRouting||0; editor.playHotkey=d.playHotkey||""; editor.assignHotkey=d.assignHotkey||""; loadWaveform()
    }
}