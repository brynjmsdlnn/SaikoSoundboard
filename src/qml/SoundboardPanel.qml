import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

Rectangle {
    id: root
    color: "#0f0f0f"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Top bar with Add and Routing buttons
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 52
            color: "#111111"
            border.color: "#1c1c1c"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 12

                Text {
                    text: "Soundboard Slots"
                    color: "white"
                    font.pixelSize: 14
                    font.weight: Font.Bold
                }

                Item { Layout.fillWidth: true }

                // Add button
                Rectangle {
                    id: addBtn
                    width: 32
                    height: 32
                    radius: 8
                    color: addMouse.containsMouse ? "#1c1c1c" : "#121212"
                    border.color: addMouse.containsMouse ? "#333" : "#1c1c1c"
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Behavior on border.color { ColorAnimation { duration: 150 } }

                    Text {
                        anchors.centerIn: parent
                        text: "+"
                        color: addMouse.containsMouse ? "white" : "#bbb"
                        font.pixelSize: 16
                        font.weight: Font.Bold
                        Behavior on color { ColorAnimation { duration: 150 } }
                    }

                    MouseArea {
                        id: addMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: qmlBackend.soundboard.addPlayer()
                    }
                }

                // Settings/Routing button
                Rectangle {
                    id: routingBtn
                    width: 32
                    height: 32
                    radius: 8
                    color: routingMouse.containsMouse ? "#1c1c1c" : "#121212"
                    border.color: routingMouse.containsMouse ? "#333" : "#1c1c1c"
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Behavior on border.color { ColorAnimation { duration: 150 } }

                    Text {
                        anchors.centerIn: parent
                        text: "⚙"
                        color: routingMouse.containsMouse ? "white" : "#bbb"
                        font.pixelSize: 15
                        Behavior on color { ColorAnimation { duration: 150 } }
                    }

                    MouseArea {
                        id: routingMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: openRoutingDialog()
                    }
                }
            }
        }

        // Scrollable card area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#0f0f0f"

            ScrollView {
                anchors.fill: parent
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AlwaysOff
                ScrollBar.horizontal: ScrollBar {
                    height: 6
                    policy: ScrollBar.AsNeeded
                    background: Rectangle { color: "transparent" }
                    contentItem: Rectangle {
                        color: parent.pressed ? "#BB86FC" : (parent.hovered ? "#888" : "#222")
                        radius: 3
                        Behavior on color { ColorAnimation { duration: 150 } }
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

                    model: soundboardSlotModel
                    delegate: SoundboardCard {
                        slotModel: soundboardSlotModel
                        slotIndex: index
                    }
                }
            }
        }
    }

    function openRoutingDialog() {
        var component = Qt.createComponent("qrc:/qml/RoutingDialog.qml")
        if (component.status === Component.Ready) {
            var win = component.createObject(null, {})
            win.accepted.connect(function() { win.close() })
            win.rejected.connect(function() { win.close() })
            win.show()
        } else {
            if (component.status === Component.Error)
                console.error("RoutingDialog error:", component.errorString())
        }
    }
}
