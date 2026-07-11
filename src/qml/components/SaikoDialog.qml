import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Popup {
    id: root
    parent: Overlay.overlay
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    width: 340
    modal: true
    dim: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    padding: 20

    property string title: ""
    property string text: ""
    property string informativeText: ""
    
    property string confirmText: "OK"
    property string cancelText: "Cancel"
    property color confirmColor: Theme.accentPurple
    property bool showCancel: true
    property bool showConfirm: true

    signal accepted()
    signal rejected()

    function accept() {
        accepted()
        close()
    }

    function reject() {
        rejected()
        close()
    }
    
    // Support default property so child elements go into our custom content area
    default property alias customContent: customContentContainer.data
    
    property real contentScale: 1.0

    background: Rectangle {
        color: Theme.cardBackground
        border.color: Theme.borderDefault
        border.width: 1
        radius: Theme.cardRadius
        scale: root.contentScale
        transformOrigin: Item.Center
        
        Rectangle {
            anchors.fill: parent
            anchors.margins: -1
            color: "transparent"
            border.color: Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.1)
            border.width: 1
            radius: Theme.cardRadius + 1
            z: -1
        }
    }

    Overlay.modal: Rectangle {
        color: Qt.rgba(0, 0, 0, 0.6)
    }

    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: Theme.animDuration; easing.type: Easing.OutQuad }
            NumberAnimation { target: root; property: "contentScale"; from: 0.95; to: 1.0; duration: Theme.animDuration; easing.type: Easing.OutQuad }
        }
    }
    
    exit: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: Theme.animDuration; easing.type: Easing.OutQuad }
            NumberAnimation { target: root; property: "contentScale"; from: 1.0; to: 0.95; duration: Theme.animDuration; easing.type: Easing.OutQuad }
        }
    }

    contentItem: ColumnLayout {
        spacing: 16
        scale: root.contentScale
        transformOrigin: Item.Center

        Text {
            text: root.title
            color: Theme.textPrimary
            font.pixelSize: 14
            font.weight: Font.Bold
            Layout.fillWidth: true
            visible: root.title !== ""
        }

        Text {
            text: root.text
            color: Theme.textSecondary
            font.pixelSize: 12
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            visible: root.text !== ""
        }

        Text {
            text: root.informativeText
            color: Theme.textDim
            font.pixelSize: 11
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            visible: root.informativeText !== ""
        }

        ColumnLayout {
            id: customContentContainer
            Layout.fillWidth: true
            spacing: 12
            // Custom elements go here
        }

        RowLayout {
            id: buttonsRow
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight
            spacing: 8
            Layout.topMargin: 4

            SaikoButton {
                text: root.cancelText
                small: true
                visible: root.showCancel
                onClicked: {
                    root.rejected()
                    root.close()
                }
            }

            SaikoButton {
                text: root.confirmText
                accentColor: root.confirmColor
                filled: true
                small: true
                visible: root.showConfirm
                onClicked: {
                    root.accepted()
                    root.close()
                }
            }
        }
    }
}
