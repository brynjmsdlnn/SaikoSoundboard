import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Item {
    id: root

    /// Whether the overlay is active (visible + interactive-blocking).
    property bool active: false
    /// The heading text shown in the overlay card.
    property string title: "No Audio Sources"
    /// Explanatory body text.
    property string description: "Multi-track capture requires at least one audio source before recording or replay buffer can begin."
    /// Label for the action button.
    property string buttonText: "Add Audio Source"

    /// Emitted when the user clicks the action button.
    signal actionRequested()

    anchors.fill: parent

    visible: opacity > 0.0
    opacity: root.active ? 1.0 : 0.0

    Behavior on opacity {
        NumberAnimation {
            duration: 180
            easing.type: Easing.InOutQuad
        }
    }

    // Block mouse clicks on the content underneath
    MouseArea {
        anchors.fill: parent
        enabled: root.active
    }

    // Dimmed backdrop
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.45)
        radius: 6
    }

    // Centered card
    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.72, 280)
        height: cardColumn.implicitHeight + 40

        color: Qt.rgba(22 / 255, 22 / 255, 26 / 255, 0.95)
        radius: 10
        border.color: Theme.borderDefault
        border.width: 1

        ColumnLayout {
            id: cardColumn
            anchors.centerIn: parent
            width: parent.width - 32
            spacing: 12

            Image {
                source: "image://icons/volume-x?color=%23888888"
                sourceSize: Qt.size(32, 32)
                Layout.alignment: Qt.AlignHCenter
                opacity: 0.7
            }

            Text {
                text: root.title
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeHeading
                font.weight: Font.DemiBold
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Text {
                text: root.description
                color: Theme.textDim
                font.pixelSize: Theme.fontSizeSmall
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                lineHeight: 1.3
            }

            Item { height: 4; width: 1 }

            SaikoButton {
                id: actionBtn
                text: root.buttonText
                iconSource: "image://icons/plus"
                Layout.fillWidth: true
                Layout.preferredHeight: 32

                onClicked: root.actionRequested()
            }
        }
    }
}
