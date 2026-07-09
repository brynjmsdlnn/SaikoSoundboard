import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0
import "../shared/utils.js" as Utils

Rectangle {
    id: root
    color: Theme.appBackground
    property int selectedIndex: 0

    // Global playback mode data (matches EditorHeader but without "Default" mode)
    readonly property var globalModeList: [
        { icon: "refresh-cw",         label: "Restart (Retrigger)" },
        { icon: "toggle-left",        label: "Toggle Play / Stop" },
        { icon: "list-ordered",       label: "Queued Replay (Sequential)" },
        { icon: "square-stack",      label: "Layered Play (Cut All on Stop)" },
        { icon: "audio-lines",       label: "Layered Play (Let Ring Out)" }
    ]
    readonly property int _globalModeIndex: Math.max(0, Backend.settings.defaultPlaybackMode - 1)
    readonly property string _globalModeLabel: {
        var idx = root._globalModeIndex;
        if (idx < 0 || idx >= root.globalModeList.length) return "";
        return root.globalModeList[idx].label;
    }

    // ── Tooltip cycling ────────────────────────────────────────────────────
    property string _tooltipDisplay: "Global Playback Mode"
    property bool _showingModeName: false

    Timer {
        id: tooltipTimer
        interval: 500
        onTriggered: {
            if (root._showingModeName) {
                root._tooltipDisplay = "Global Playback Mode";
                root._showingModeName = false;
                tooltipTimer.interval = 500;
            } else {
                var idx = root._globalModeIndex;
                if (idx >= 0 && idx < root.globalModeList.length) {
                    root._tooltipDisplay = root.globalModeList[idx].label;
                }
                root._showingModeName = true;
                tooltipTimer.interval = 2000;
            }
            tooltipTimer.start();
        }
    }

    // Outer Row Layout to split the screen into Left Column and Right Column
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // LEFT COLUMN (Top Bar + Grid View together)
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // TOP BAR
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 52
                color: Theme.cardBackground
                border.color: Theme.borderDefault
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 12

                    Text {
                        text: "Soundboard Slots"
                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontSizeHeading
                        font.weight: Font.Bold
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    // Global playback mode button
                    SaikoIconButton {
                        id: globalModeBtn
                        isActive: true
                        tooltipText: root._tooltipDisplay
                        iconSource: {
                            var idx = root._globalModeIndex;
                            var list = root.globalModeList;
                            if (idx < 0 || idx >= list.length) idx = 0;
                            return "image://icons/" + list[idx].icon + "?color=%23888888";
                        }
                        onClicked: globalModeMenu.openRelativeTo(globalModeBtn, root)

                        onContainsMouseChanged: {
                            if (globalModeBtn.containsMouse) {
                                root._showingModeName = false;
                                root._tooltipDisplay = "Global Playback Mode";
                                tooltipTimer.interval = 500;
                                tooltipTimer.start();
                            } else {
                                tooltipTimer.stop();
                                root._showingModeName = false;
                                root._tooltipDisplay = "Global Playback Mode";
                            }
                        }
                    }

                    SaikoIconMenu {
                        id: globalModeMenu
                        model: root.globalModeList
                        currentIndex: root._globalModeIndex
                        onActivated: function(index) {
                            Backend.settings.defaultPlaybackMode = index + 1
                            Backend.settings.save()
                        }
                    }

                    // Hotkey toggle button
                    SaikoIconButton {
                        id: hotkeyToggleBtn
                        isActive: true
                        tooltipText: Backend.settings.hotkeysEnabled ? "Disable Global Hotkeys" : "Enable Global Hotkeys"
                        iconSource: "image://icons/keyboard?color=" + (Backend.settings.hotkeysEnabled ? encodeURIComponent(Theme.accentPurple) : encodeURIComponent(Theme.textDim))
                        hoverBorderColor: Backend.settings.hotkeysEnabled ? Theme.accentPurple : Theme.borderHover
                        onClicked: {
                            Backend.settings.hotkeysEnabled = !Backend.settings.hotkeysEnabled;
                            Backend.settings.save();
                        }
                    }

                    // Add button
                    SaikoIconButton {
                        id: addBtn
                        isActive: true
                        tooltipText: "Add Slot"
                        iconSource: "image://icons/plus?color=%23b0b0b0"
                        onClicked: Backend.soundboard.addPlayer()
                    }

                    // Settings/Routing button
                    SaikoIconButton {
                        id: routingBtn
                        isActive: true
                        tooltipText: "Routing Settings"
                        iconSource: "image://icons/cog?color=%23b0b0b0"
                        onClicked: Utils.openDialog("../dialogs/RoutingDialog.qml")
                    }
                }
            }

            // GRID AREA
            GridView {
                id: grid
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                cellHeight: 140
                leftMargin: 12
                rightMargin: 12
                topMargin: 12
                bottomMargin: 12

                property int minCardWidth: 210
                property int cardSpacing: 12
                cellWidth: {
                    var available = width - leftMargin - rightMargin;
                    var cols = Math.max(3, Math.floor(available / (minCardWidth + cardSpacing)));
                    return Math.floor(available / cols);
                }

                model: SlotModel

                delegate: SoundboardGridCard {
                    width: grid.cellWidth - grid.cardSpacing
                    height: grid.cellHeight - grid.cardSpacing
                    slotModel: SlotModel
                    slotIndex: index
                    isSelected: index === root.selectedIndex
                    onClicked: root.selectedIndex = index

                    slotName: model.slotName
                    durationSec: model.durationSec
                    outputRouting: model.outputRouting
                    playbackMode: model.playbackMode
                    slotId: model.slotId
                    filePath: model.filePath
                    locked: model.locked
                    fileExists: model.fileExists
                    startTimeMs: model.startTimeMs
                    endTimeMs: model.endTimeMs
                    playState: model.playState
                }
            }
        }

        // RIGHT COLUMN
        SlotEditor {
            id: rightEditor
            Layout.preferredWidth: 500
            Layout.minimumWidth: 500
            Layout.maximumWidth: 520
            Layout.fillHeight: true
            Layout.fillWidth: false
            slotModel: SlotModel
            slotIndex: root.selectedIndex
        }
    }
}
