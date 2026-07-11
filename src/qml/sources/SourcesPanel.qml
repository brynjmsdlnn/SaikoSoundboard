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
    signal deviceAdded(string name, string deviceName)
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
                property bool isExpanded: listView.currentIndex === index
                height: isExpanded ? 90 : 48
                radius: Theme.cardRadius
                color: listView.currentIndex === index ? "#161616" : (mouseArea.containsMouse ? "#121212" : "#101010")
                border.color: listView.currentIndex === index ? Theme.accentPurple : (mouseArea.containsMouse ? Theme.borderHover : Theme.borderDefault)
                border.width: 1
                clip: true

                Behavior on height { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                Behavior on color { ColorAnimation { duration: Theme.animDuration } }
                Behavior on border.color { ColorAnimation { duration: Theme.animDuration } }

                // FIX 1: Restrict MouseArea to the header only so it doesn't block the slider
                MouseArea {
                    id: mouseArea
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 48 
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: listView.currentIndex = (listView.currentIndex === index ? -1 : index)
                }

                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 0

                    // Top Header Row
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 48
                        Layout.leftMargin: 12
                        Layout.rightMargin: 12
                        spacing: 10

                        // Process Icon
                        Image {
                            source: {
                                if (model.type === "device") {
                                    return "image://icons/volume-2?color=%23b0b0b0"
                                }
                                return executablePath ? "image://fileicon/" + encodeURIComponent(executablePath) : ""
                            }
                            width: 24
                            height: 24
                            fillMode: Image.PreserveAspectFit
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            opacity: (model.type === "device" || executablePath) ? 0.9 : 0.2
                            visible: model.type === "device" || executablePath !== ""
                        }

                        // Process Details
                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 0
                            Layout.minimumWidth: 0
                            spacing: 2

                            Text {
                                text: name
                                color: Theme.textPrimary
                                font.pixelSize: Theme.fontSizeHeading
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                            Text {
                                text: model.type === "device" ? model.deviceName : executableName
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSizeSmall
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }

                        // Actions (Mute & Solo)
                        RowLayout {
                            spacing: 6
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

                            // Mute/Unmute Toggle
                            Item {
                                width: 28
                                height: 22
                                SaikoButton {
                                    id: muteBtn
                                    anchors.fill: parent
                                    iconSource: model.enabled ? "image://icons/volume-2" : "image://icons/volume-x"
                                    small: true
                                    filled: !model.enabled
                                    accentColor: Theme.accentTeal
                                    onClicked: {
                                        var newEnabled = !model.enabled
                                        var sourceId = root.sourceModel.getSourceId(index)
                                        root.sourceModel.setEnabled(sourceId, newEnabled)
                                        Backend.recording.setSourceMuted(sourceId, !newEnabled)
                                    }
                                }
                                SaikoTooltip {
                                    text: model.enabled ? "Mute Source (Exclude from Recording)" : "Unmute Source (Include in Recording)"
                                    hovered: muteBtn.hovered
                                    direction: "left"
                                    z: 999
                                }
                            }

                            // Listen/Passthrough Toggle (Only for Device type sources)
                            Item {
                                width: 28
                                height: 22
                                visible: !root.locked && model.type === "device"
                                SaikoButton {
                                    id: listenBtn
                                    anchors.fill: parent
                                    iconSource: "image://icons/headset"
                                    small: true
                                    filled: model.monitor
                                    accentColor: Theme.accentPurple
                                    onClicked: {
                                        var newMonitor = !model.monitor
                                        var sourceId = root.sourceModel.getSourceId(index)
                                        root.sourceModel.setMonitor(sourceId, newMonitor)
                                    }
                                }
                                SaikoTooltip {
                                    text: model.monitor ? "Stop Listening (Mute Monitoring)" : "Listen to Device (Passthrough to Headphones)"
                                    hovered: listenBtn.hovered
                                    direction: "left"
                                    z: 999
                                }
                            }

                            // Solo Toggle
                            Item {
                                width: 28
                                height: 22
                                visible: !root.locked
                                SaikoButton {
                                    id: soloBtn
                                    anchors.fill: parent
                                    iconSource: "image://icons/headphones"
                                    small: true
                                    filled: solo
                                    accentColor: Theme.accentTeal
                                    onClicked: {
                                        var newSolo = !solo
                                        var sourceId = root.sourceModel.getSourceId(index)
                                        root.sourceModel.setSolo(sourceId, newSolo)
                                        Backend.recording.setSourceSolo(sourceId, newSolo)
                                    }
                                }
                                SaikoTooltip {
                                    text: "Solo Monitor — hear only this source"
                                    hovered: soloBtn.hovered
                                    direction: "left"
                                    z: 999
                                }
                            }
                        }
                    }

                    // Bottom Expandable Volume Controls
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        Layout.leftMargin: 12
                        Layout.rightMargin: 12
                        Layout.bottomMargin: 4
                        spacing: 8
                        
                        // FIX 2: Drop the opacity to 40% if muted (model.enabled is false)
                        opacity: itemCard.isExpanded ? (model.enabled ? 1.0 : 0.4) : 0.0

                        Behavior on opacity { NumberAnimation { duration: 150 } }

                        Image {
                            source: "image://icons/sliders-horizontal?color=%23888888"
                            width: 16
                            height: 16
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                        }

                        Slider {
                            id: volumeSlider
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: model.volume * 100
                            live: true

                            background: Rectangle {
                                x: volumeSlider.leftPadding
                                y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                                width: volumeSlider.availableWidth
                                height: 4
                                radius: 2
                                color: Theme.borderDefault // Using your theme's default border color like the working example

                                Rectangle {
                                    width: volumeSlider.visualPosition * parent.width
                                    height: parent.height
                                    color: model.enabled ? Theme.accentPurple : "#666666"
                                    radius: 2
                                }
                            }

                            handle: Rectangle {
                                x: volumeSlider.leftPadding + volumeSlider.visualPosition * (volumeSlider.availableWidth - width)
                                y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2

                                // FIX 1: Explicit dimensions matching your working slider
                                width: 14
                                height: 14
                                radius: 7

                                color: volumeSlider.pressed ? Theme.accentPurple : (model.enabled ? Theme.textPrimary : "#888888")
                                border.color: model.enabled ? Theme.accentPurple : "#666666"
                                border.width: 1
                            }

                            // FIX 2: Split the update logic
                            onMoved: {
                                // 1. Update the actual audio engine live so the user hears the change immediately
                                var newVol = value / 100.0
                                var sourceId = root.sourceModel.getSourceId(index)
                                Backend.recording.setSourceVolume(sourceId, newVol)
                            }

                            onPressedChanged: {
                                if (!pressed) {
                                    // 2. Update the UI ListModel ONLY when the mouse is released.
                                    // This prevents the ListView from refreshing and killing the drag!
                                    var newVol = value / 100.0
                                    var sourceId = root.sourceModel.getSourceId(index)
                                    root.sourceModel.setVolume(sourceId, newVol)
                                }
                            }
                        }

                        Text {
                            // FIX 3: Bind text directly to the slider so it updates live without needing the model
                            text: volumeSlider.value.toFixed(0) + "%"

                            color: model.enabled ? Theme.textDim : "#666666"
                            font.pixelSize: Theme.fontSizeSmall
                            font.weight: Font.DemiBold
                            Layout.preferredWidth: 36
                            horizontalAlignment: Text.AlignRight
                        }
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
                text: "Add Source"
                iconSource: "image://icons/plus"
                Layout.fillWidth: true
                enabled: !root.locked
                onClicked: processPopup.open()
            }

            SaikoButton {
                id: removeBtn
                text: "Remove"
                iconSource: "image://icons/trash-2"
                Layout.fillWidth: true
                accentColor: Theme.destructiveRed
                enabled: !root.locked && listView.currentIndex >= 0
                onClicked: {
                    var idx = listView.currentIndex
                    if (idx >= 0) {
                        var id = root.sourceModel.getSourceId(idx)
                        if (id) {
                            Backend.recording.setSourceSolo(id, false)
                            Backend.recording.setSourceMuted(id, true)
                            root.sourceRemoved(id)
                        }
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
        onDeviceSelected: (name, deviceName) => {
            root.deviceAdded(name, deviceName)
        }
    }
}
