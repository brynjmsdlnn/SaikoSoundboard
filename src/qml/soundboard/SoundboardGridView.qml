import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0
import "../shared/utils.js" as Utils

Rectangle {
    id: root
    color: Theme.appBackground
    property int selectedIndex: 0

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

                    // Add button
                    Item {
                        width: 32
                        height: 28
                        SaikoButton {
                            id: addBtn
                            anchors.fill: parent
                            iconSource: "image://icons/plus?color=%23b0b0b0"
                            small: true
                            onClicked: Backend.soundboard.addPlayer()
                        }
                        SaikoTooltip {
                            text: "Add Slot"
                            hovered: addBtn.hovered
                            direction: "top"
                            z: 999
                        }
                    }

                    // Settings/Routing button
                    Item {
                        width: 32
                        height: 28
                        SaikoButton {
                            id: routingBtn
                            anchors.fill: parent
                            iconSource: "image://icons/cog?color=%23b0b0b0"
                            small: true
                            onClicked: Utils.openDialog("../dialogs/RoutingDialog.qml")
                        }
                        SaikoTooltip {
                            text: "Routing Settings"
                            hovered: routingBtn.hovered
                            direction: "top"
                            z: 999
                        }
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
                    slotId: model.slotId
                    filePath: model.filePath
                    locked: model.locked
                }
            }
        }

        // RIGHT COLUMN
        ItemEditor {
            id: rightEditor
            Layout.preferredWidth: 500
            Layout.minimumWidth: 480
            Layout.maximumWidth: 520
            Layout.fillHeight: true
            Layout.fillWidth: false
            slotModel: SlotModel
            slotIndex: root.selectedIndex
        }
    }
}
