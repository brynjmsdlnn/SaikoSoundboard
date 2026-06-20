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

    function isModifier(key) {
        return key === Qt.Key_Control || key === Qt.Key_Shift ||
               key === Qt.Key_Alt || key === Qt.Key_Meta
    }
    function keyName(key) {
        if (key === Qt.Key_Space) return "SPACE"
        if (key === Qt.Key_Return) return "ENTER"
        if (key === Qt.Key_Tab) return "TAB"
        if (key === Qt.Key_Escape) return "ESC"
        if (key >= Qt.Key_0 && key <= Qt.Key_9) return String.fromCharCode(key)
        if (key >= Qt.Key_A && key <= Qt.Key_Z) return String.fromCharCode(key).toUpperCase()
        return "KEY"
    }
    function getSequence(event) {
        var parts = []
        if (event.modifiers & Qt.ControlModifier) parts.push("CTRL")
        if (event.modifiers & Qt.ShiftModifier) parts.push("SHIFT")
        if (event.modifiers & Qt.AltModifier) parts.push("ALT")
        parts.push(root.keyName(event.key))
        return parts.join("+")
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

    component HotkeyCard : Rectangle {
        id: card
        property string title: ""
        property string currentKey: ""
        property bool isRecording: false
        property color accentColor: "white"
        property bool isHovered: false

        signal clicked()
        signal keyCaptured(string sequence)

        Layout.fillWidth: true
        Layout.fillHeight: true
        radius: 12
        color: isRecording ? "#161616" : (isHovered ? "#141414" : "#111111")
        border.color: isRecording ? accentColor : (isHovered ? Theme.borderHover : Theme.borderDefault)
        border.width: 1
        Behavior on color { ColorAnimation { duration: 180 } }
        Behavior on border.color { ColorAnimation { duration: 180 } }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onEntered: card.isHovered = true
            onExited: card.isHovered = false
            onClicked: { card.forceActiveFocus(); card.clicked() }
        }

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 14

            Text {
                text: card.title
                Layout.alignment: Qt.AlignHCenter
                font.pixelSize: 10
                font.letterSpacing: 2.5
                color: card.isRecording ? card.accentColor : Theme.textDim
                Behavior on color { ColorAnimation { duration: 180 } }
            }

            Text {
                text: card.isRecording ? "· · ·" : card.currentKey
                color: card.isRecording ? card.accentColor : Theme.textPrimary
                font.pixelSize: card.currentKey.length > 5 ? 22 : 30
                font.weight: Font.Bold
                Layout.alignment: Qt.AlignHCenter
                Behavior on color { ColorAnimation { duration: 180 } }

                SequentialAnimation on opacity {
                    running: card.isRecording
                    loops: Animation.Infinite
                    NumberAnimation { from: 1.0; to: 0.2; duration: 550; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 0.2; to: 1.0; duration: 550; easing.type: Easing.InOutQuad }
                }
            }

            Text {
                text: card.isRecording ? "PRESS ANY KEY" : "CLICK TO EDIT"
                Layout.alignment: Qt.AlignHCenter
                font.pixelSize: 9
                font.letterSpacing: 1.5
                color: card.isRecording ? card.accentColor : Theme.textDim
                opacity: card.isHovered && !card.isRecording ? 0.8 : (card.isRecording ? 1.0 : 0.4)
                Behavior on color { ColorAnimation { duration: 180 } }
                Behavior on opacity { NumberAnimation { duration: 180 } }
            }
        }

        Keys.onPressed: (event) => {
            if (card.isRecording && !root.isModifier(event.key)) {
                card.keyCaptured(root.getSequence(event))
                event.accepted = true
            }
        }
    }
}