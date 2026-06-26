import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Rectangle {
    id: root

    property bool modeEnabled: true
    property string captureMode: "global"
    property int cardPadding: 12

    signal captureModeSelected(string newMode)
    signal settingsRequested

    implicitHeight: captureContent.implicitHeight + 24
    color: Theme.appBackground
    radius: Theme.cardRadius
    border.color: Theme.borderDefault

    RowLayout {
        id: captureContent
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: root.cardPadding
        spacing: 10

        Text {
            text: "Capture Mode:"
            color: Theme.textSecondary
            font.bold: true
        }

        SaikoComboBox {
            Layout.preferredWidth: 220
            model: [
                { text: "System Output (Global)", value: "global" },
                { text: "Multi-track (sources)", value: "multi" }
            ]
            textRole: "text"
            valueRole: "value"
            isActive: root.modeEnabled
            currentIndex: root.captureMode === "multi" ? 1 : 0
            onActivated: {
                root.captureModeSelected(currentValue);
            }
        }

        Item {
            Layout.fillWidth: true
        }

        SaikoButton {
            iconSource: "image://icons/folder?color=%23b0b0b0"
            text: "Recordings"
            small: true
            implicitWidth: 110
            onClicked: Qt.openUrlExternally("file:///" + encodeURI(Backend.settings.recordingDirectory))
        }

        SaikoButton {
            iconSource: "image://icons/folder?color=%23b0b0b0"
            text: "Replays"
            small: true
            implicitWidth: 110
            onClicked: Qt.openUrlExternally("file:///" + encodeURI(Backend.settings.replayDirectory))
        }

        SaikoButton {
            iconSource: "image://icons/cog?color=%23b0b0b0"
            small: true
            implicitWidth: 28
            onClicked: root.settingsRequested()
        }
    }
}
