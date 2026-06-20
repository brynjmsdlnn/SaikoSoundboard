import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Saiko 1.0
import "utils.js" as Utils

Rectangle {
    id: root
            color: Theme.appBackground

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Top bar with Add and Routing buttons
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

                Item { Layout.fillWidth: true }

                // Add button
                ThemedButton {
                    id: addBtn
                    text: "+"
                    small: true
                    implicitWidth: 32
                    onClicked: Backend.soundboard.addPlayer()
                }

                // Settings/Routing button
                ThemedButton {
                    id: routingBtn
                    text: "\u2699"
                    small: true
                    implicitWidth: 32
                    onClicked: openRoutingDialog()
                }
            }
        }

        // Scrollable card area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
    color: Theme.appBackground

            ScrollView {
                anchors.fill: parent
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AlwaysOff
            ScrollBar.horizontal: ScrollBar {
                height: 6
                policy: ScrollBar.AsNeeded
                background: Rectangle { color: "transparent" }
                contentItem: Rectangle {
                    color: parent.pressed ? Theme.accentPurple : (parent.hovered ? Theme.textDim : Theme.borderDefault)
                    radius: 3
                    Behavior on color { ColorAnimation { duration: Theme.animDuration } }
                }
            }

                ListView {
                    id: cardList
                    anchors.fill: parent
                    orientation: ListView.Horizontal
                    spacing: 14
                    topMargin: 14
                    bottomMargin: 14
                    leftMargin: 14
                    rightMargin: 14

                    model: SlotModel
                    delegate: SoundboardCard {
                        slotModel: SlotModel
                        slotIndex: index
                    }
                }
            }
        }
    }

    function openRoutingDialog() {
        Utils.openDialog("RoutingDialog.qml")
    }
}
