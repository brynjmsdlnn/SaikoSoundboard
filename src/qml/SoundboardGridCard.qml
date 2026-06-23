import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Saiko 1.0

Rectangle {
    id: card
    radius: 8
    color: isSelected ? "#19141f" : (cardHover.hovered ? "#161616" : Theme.cardBackground)
    border.color: isSelected ? Theme.accentPurple : (cardHover.hovered ? Theme.borderHover : Theme.borderDefault)
    border.width: isSelected ? 2 : 1

    Behavior on color { ColorAnimation { duration: Theme.animDuration } }
    Behavior on border.color { ColorAnimation { duration: Theme.animDuration } }

    property var slotModel
    property int slotIndex: -1
    property bool isSelected: false
    signal clicked()

    property string slotName
    property real durationSec
    property int outputRouting
    property string slotId
    property string filePath

    property var waveformData: null

    HoverHandler {
        id: cardHover
    }

    MouseArea {
        id: clickArea
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton) {
                contextMenu.popup()
            } else {
                card.clicked()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4

        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "#" + ("00" + (slotIndex + 1)).slice(-2)
                color: Theme.textDim
                font.pixelSize: Theme.fontSizeSmall
            }
            Item { Layout.fillWidth: true }
            Text {
                text: durationSec ? durationSec.toFixed(1) + "s" : "0.0s"
                color: Theme.textDim
                font.pixelSize: Theme.fontSizeSmall
            }
        }

        Text {
            text: slotName || "Empty Slot"
            color: slotName ? Theme.textPrimary : Theme.textDim
            font.pixelSize: Theme.fontSizeNormal
            font.weight: Font.Medium
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        // mini waveform
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "transparent"
            clip: true

            WaveformView {
                id: miniWaveform
                anchors.fill: parent
                readOnly: true
                startMs: 0
                endMs: durationSec * 1000
                waveformData: card.waveformData
            }
        }

        RowLayout {
            spacing: 8
            Layout.fillWidth: true

            Rectangle {
                width: 20
                height: 20
                radius: 4
                color: playMouse.containsMouse ? Theme.accentPurple : Theme.inputBackground
                border.color: playMouse.containsMouse ? Theme.accentPurple : Theme.borderDefault
                border.width: 1
                Behavior on color { ColorAnimation { duration: 100 } }

                Image {
                    anchors.centerIn: parent
                    source: "image://icons/play?color=%23" + (playMouse.containsMouse ? "1e1e1e" : "b0b0b0")
                    width: 10
                    height: 10
                }

                MouseArea {
                    id: playMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (slotId) Backend.soundboard.playPlayer(slotId)
                    }
                }
            }

            Text {
                text: {
                    if (outputRouting === 0) return "Broadcast & Monitor"
                    if (outputRouting === 1) return "Broadcast only"
                    return "Monitor only"
                }
                color: {
                    if (outputRouting === 0) return Theme.accentPurple
                    if (outputRouting === 1) return Theme.accentTeal
                    return Theme.accentGreen
                }
                font.pixelSize: 9
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }
    }

    Connections {
        target: Backend.soundboard
        function onWaveformGenerated(playerId, data) {
            if (playerId === slotId && data.isValid) {
                card.waveformData = data
            }
        }
        function onPlayerPositionChanged(playerId, position) {
            if (playerId === slotId) {
                miniWaveform.playPositionMs = position
            }
        }
    }

    onFilePathChanged: {
        loadWaveform()
    }

    Component.onCompleted: {
        loadWaveform()
    }

    function loadWaveform() {
        if (filePath && filePath !== "") {
            var wfData = Backend.soundboard.getWaveformData(slotId)
            if (wfData && wfData.isValid) {
                card.waveformData = wfData
            } else {
                Backend.soundboard.loadWaveformData(slotId, filePath)
            }
        } else {
            card.waveformData = null
        }
    }

    CardMenu {
        id: contextMenu
        CardMenuItem {
            text: "Assign from file..."
            onClicked: assignFileDialog.open()
        }
        CardMenuItem {
            text: "Assign from replay buffer"
            onClicked: {
                if (slotId) {
                    Backend.actions.dispatchAssignReplay(slotId, false)
                }
            }
        }
        CardMenuItem {
            text: "Delete slot"
            onClicked: removeConfirmDialog.open()
        }
    }

    FileDialog {
        id: assignFileDialog
        title: "Select audio file"
        nameFilters: ["Audio files (*.wav *.mp3 *.ogg)"]
        onAccepted: {
            if (selectedFile && slotId) {
                Backend.soundboard.assignAudioFile(slotId, selectedFile)
            }
        }
    }

    MessageDialog {
        id: removeConfirmDialog
        title: "Confirm delete"
        text: "Are you sure you want to delete this player?"
        informativeText: (filePath && filePath !== "") ?
            "It has an assigned audio file." : ""
        buttons: MessageDialog.Yes | MessageDialog.No
        onAccepted: {
            if (slotId) {
                Backend.soundboard.removePlayer(slotId)
            }
        }
    }
}