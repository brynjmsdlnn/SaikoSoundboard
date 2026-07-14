import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Rectangle {
    id: root

    property bool startEnabled: true
    property int cardPadding: 12

    // Capture mode properties
    property bool modeEnabled: true
    property string captureMode: "global"

    property bool logViewerActive: false
    signal captureModeSelected(string newMode)
    signal settingsRequested()
    signal aboutRequested()
    signal toggleLogViewerRequested()

    implicitHeight: headerContent.implicitHeight + 24
    radius: Theme.cardRadius
    border.color: Theme.borderDefault
    color: Theme.appBackground

    readonly property var captureModeList: [
        { label: "System Output (Global)", value: "global", icon: "monitor", modeColor: "#ffffff", index: 0 },
        { label: "Multi-track (sources)", value: "multi", icon: "layers", modeColor: "#bb86fc", index: 1 }
    ]

    // Properties for icon cycling animation
    property bool _showWaveformIcon: false

    Timer {
        id: iconCycleTimer
        interval: 2500
        repeat: true
        running: !rightUtilities.isExpanded
        onTriggered: {
            root._showWaveformIcon = !root._showWaveformIcon;
        }
        onRunningChanged: {
            if (!running) {
                root._showWaveformIcon = false; // Force showing the active mode's icon when hovered/expanded
            }
        }
    }

    RowLayout {
        id: headerContent
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: root.cardPadding
        spacing: 12

        // Left Logo Icon and Title
        RowLayout {
            spacing: 8
            Layout.alignment: Qt.AlignVCenter

            Image {
                id: headerIcon
                source: "image://icons/radio?color=%23" + (headerIcon.isActive ? (Backend.isPlaying ? "bb86fc" : "e35d5d") : "b0b0b0")
                width: 16
                height: 16

                property bool isActive: Backend.isPlaying || !root.startEnabled

                SequentialAnimation on opacity {
                    running: headerIcon.isActive
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.2; duration: 800; easing.type: Easing.InOutQuad }
                    NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
                }

                onIsActiveChanged: {
                    if (!isActive)
                        opacity = 1.0;
                }
            }

            Text {
                text: "Saiko Soundboard"
                color: Theme.textPrimary
                font.pixelSize: 13
                font.weight: Font.DemiBold
            }

            Rectangle {
                height: 16
                implicitWidth: versionText.implicitWidth + 8
                color: Qt.rgba(Theme.accentTeal.r, Theme.accentTeal.g, Theme.accentTeal.b, 0.15)
                border.color: Theme.accentTeal
                border.width: 1
                radius: 8
                Layout.alignment: Qt.AlignVCenter

                Text {
                    id: versionText
                    anchors.centerIn: parent
                    text: "v0.2.0"
                    color: Theme.accentTeal
                    font.pixelSize: 9
                    font.weight: Font.Bold
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }

        // Right Utilities: Capture Mode Selector and Settings/About Icon Buttons
        RowLayout {
            id: rightUtilities
            spacing: 8
            Layout.alignment: Qt.AlignVCenter

            readonly property bool isExpanded: captureModeBtn.containsMouse || labelMouseArea.containsMouse

            // Expanding / Collapsing Label Row (on the left of the button)
            Item {
                id: labelRow
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: rightUtilities.isExpanded ? labelLayout.implicitWidth : 0
                height: labelLayout.implicitHeight
                opacity: rightUtilities.isExpanded ? 1.0 : 0.0
                visible: opacity > 0.0
                clip: true

                Behavior on Layout.preferredWidth {
                    NumberAnimation { duration: 250; easing.type: Easing.InOutQuad }
                }
                Behavior on opacity {
                    NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
                }

                MouseArea {
                    id: labelMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.NoButton
                }

                RowLayout {
                    id: labelLayout
                    anchors.fill: parent
                    spacing: 6

                    Image {
                        source: "image://icons/audio-waveform?color=%23b0b0b0"
                        sourceSize: Qt.size(14, 14)
                        Layout.alignment: Qt.AlignVCenter
                    }
                    Text {
                        text: "Capture Mode:"
                        color: Theme.textSecondary
                        font.bold: true
                        font.pixelSize: 12
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            // Capture Mode Icon Button (on the right of the label - stays stationary)
            SaikoIconButton {
                id: captureModeBtn
                isActive: root.modeEnabled
                tooltipText: "" // Disabled since we show the label on hover instead
                
                iconSource: {
                    if (root._showWaveformIcon) {
                        return "image://icons/audio-waveform?color=%23b0b0b0";
                    }
                    var idx = root.captureMode === "multi" ? 1 : 0;
                    var item = root.captureModeList[idx];
                    return "image://icons/" + item.icon + "?color=%23" + item.modeColor.replace("#", "");
                }
                onClicked: captureModeMenu.openRelativeTo(captureModeBtn, root)
            }

            SaikoIconButton {
                iconSource: "image://icons/cog?color=%23b0b0b0"
                tooltipText: "Settings"
                tooltipDirection: "bottom"
                onClicked: root.settingsRequested()
            }

            SaikoIconButton {
                iconSource: "image://icons/info?color=%23b0b0b0"
                tooltipText: "About"
                tooltipDirection: "bottom"
                onClicked: root.aboutRequested()
            }

            // Log viewer toggle button
            SaikoIconButton {
                id: logViewerBtn
                isActive: true
                tooltipText: root.logViewerActive ? "Hide Logs" : "Show Logs"
                tooltipDirection: "bottom"
                iconSource: "image://icons/list?color=" + encodeURIComponent(root.logViewerActive ? Theme.accentPurple : Theme.textDim)
                onClicked: root.toggleLogViewerRequested()
            }
        }
    }

    // Declared outside layouts as a direct child of root to ensure mapToItem coordinate parent works correctly
    SaikoIconMenu {
        id: captureModeMenu
        model: root.captureModeList
        currentIndex: root.captureMode === "multi" ? 1 : 0
        onActivated: function (index) {
            var selectedMode = root.captureModeList[index].value;
            root.captureModeSelected(selectedMode);
        }
    }
}
