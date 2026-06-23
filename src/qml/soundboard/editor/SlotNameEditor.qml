import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

ColumnLayout {
    id: root

    property string slotName: ""
    property string slotId: ""
    property bool isLocked: false

    Layout.fillWidth: true
    spacing: 8

    SectionLabel {
        text: "SLOT NAME"
    }

    TextField {
        id: nameField
        Layout.fillWidth: true
        implicitHeight: 36
        text: root.slotName
        placeholderText: "Unnamed Slot"
        color: Theme.textPrimary
        font.pixelSize: 14
        selectByMouse: true
        rightPadding: 32

        property bool showSuccess: false
        property bool isDirty: activeFocus && text !== root.slotName

        SequentialAnimation {
            id: successAnim
            PropertyAction { target: nameField; property: "showSuccess"; value: true }
            PauseAnimation { duration: 1500 }
            PropertyAction { target: nameField; property: "showSuccess"; value: false }
        }

        background: Rectangle {
            color: Theme.inputBackground
            border.color: nameField.showSuccess ? Theme.accentGreen : (nameField.activeFocus ? Theme.accentPurple : Theme.borderDefault)
            radius: 6
            Behavior on border.color { ColorAnimation { duration: 200 } }
        }

        enabled: !root.isLocked

        Image {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 10
            source: "image://icons/corner-down-left?color=%23b0b0b0"
            sourceSize: Qt.size(16, 16)
            opacity: nameField.isDirty && !nameField.showSuccess ? 1.0 : 0.0
            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        Image {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 10
            source: "image://icons/check?color=%234caf50"
            sourceSize: Qt.size(16, 16)
            opacity: nameField.showSuccess ? 1.0 : 0.0
            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        onEditingFinished: {
            if (root.slotId && text.trim() !== root.slotName) {
                Backend.soundboard.renamePlayer(root.slotId, text.trim())
                successAnim.restart()
            }
        }
    }
}
