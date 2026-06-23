import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Rectangle {
    id: root
    implicitWidth: 320
    implicitHeight: 450
    color: Theme.appBackground

    property var sourceModel: null
    property bool locked: false

    signal sourceAdded(string name, string executableName, string executablePath)
    signal sourceRemoved(string sourceId)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        // List of Active Sources
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.sourceModel
            currentIndex: -1
            spacing: 6

            delegate: Rectangle {
                id: itemCard
                width: listView.width
                height: 48
                radius: Theme.cardRadius
                color: listView.currentIndex === index ? "#161616" : (mouseArea.containsMouse ? "#121212" : "#101010")
                border.color: listView.currentIndex === index ? Theme.accentPurple : (mouseArea.containsMouse ? Theme.borderHover : Theme.borderDefault)
                border.width: 1

                Behavior on color { ColorAnimation { duration: Theme.animDuration } }
                Behavior on border.color { ColorAnimation { duration: Theme.animDuration } }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: listView.currentIndex = index
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 12

                    // Process Icon
                    Image {
                        source: executablePath ? "image://fileicon/" + encodeURIComponent(executablePath) : ""
                        width: 24
                        height: 24
                        fillMode: Image.PreserveAspectFit
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        opacity: executablePath ? 0.9 : 0.2
                        visible: executablePath !== ""
                    }

                    // Process Details
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Text {
                            text: name
                            color: Theme.textPrimary
                            font.pixelSize: Theme.fontSizeHeading
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }
                        Text {
                            text: executableName
                            color: Theme.textDim
                            font.pixelSize: Theme.fontSizeSmall
                            elide: Text.ElideRight
                        }
                    }

                    // Indicator dot if selected
                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        color: Theme.accentPurple
                        visible: listView.currentIndex === index
                        Layout.alignment: Qt.AlignVCenter
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {
                width: 6
                policy: ScrollBar.AsNeeded
            }
        }

        // Custom Styled Action Buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            SaikoButton {
                id: addBtn
                text: "+ Add Source"
                Layout.fillWidth: true
                enabled: !root.locked
                onClicked: processPopup.open()
            }

            SaikoButton {
                id: removeBtn
                text: "- Remove"
                Layout.fillWidth: true
                accentColor: Theme.destructiveRed
                enabled: !root.locked && listView.currentIndex >= 0
                onClicked: {
                    var idx = listView.currentIndex
                    if (idx >= 0) {
                        var id = root.sourceModel.getSourceId(idx)
                        if (id) root.sourceRemoved(id)
                        listView.currentIndex = -1
                    }
                }
            }
        }
    }

    ProcessSelectionPopup {
        id: processPopup
        parent: root
        sourceModel: root.sourceModel
        onProcessSelected: (name, executableName, executablePath) => {
            root.sourceAdded(name, executableName, executablePath)
        }
    }
}
