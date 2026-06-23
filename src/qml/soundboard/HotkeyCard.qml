import QtQuick
import QtQuick.Layouts
import Saiko 1.0

Rectangle {
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
        parts.push(card.keyName(event.key))
        return parts.join("+")
    }

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
            text: card.isRecording ? "\u00B7 \u00B7 \u00B7" : card.currentKey
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
        if (card.isRecording && !card.isModifier(event.key)) {
            card.keyCaptured(card.getSequence(event))
            event.accepted = true
        }
    }
}
