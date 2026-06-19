import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    implicitWidth: 320
    implicitHeight: 450
    color: "#0f0f0f"

    property var sourceModel: []
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
                radius: 8
                color: listView.currentIndex === index ? "#161616" : (mouseArea.containsMouse ? "#121212" : "#101010")
                border.color: listView.currentIndex === index ? "#BB86FC" : (mouseArea.containsMouse ? "#2a2a2a" : "#1a1a1a")
                border.width: 1

                Behavior on color { ColorAnimation { duration: 150 } }
                Behavior on border.color { ColorAnimation { duration: 150 } }

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
                        source: modelData.executablePath ? "image://fileicon/" + encodeURIComponent(modelData.executablePath) : ""
                        width: 24
                        height: 24
                        fillMode: Image.PreserveAspectFit
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        opacity: modelData.executablePath ? 0.9 : 0.2
                        visible: modelData.executablePath !== ""
                    }

                    // Process Details
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Text {
                            text: modelData.name
                            color: "white"
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }
                        Text {
                            text: modelData.executableName
                            color: "#555"
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }
                    }

                    // Indicator dot if selected
                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        color: "#BB86FC"
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

            Rectangle {
                id: addBtn
                Layout.fillWidth: true
                height: 38
                radius: 8
                color: !root.locked && addMouse.containsMouse ? "#1c1c1c" : "#121212"
                border.color: !root.locked && addMouse.containsMouse ? "#333" : "#1c1c1c"
                border.width: 1
                opacity: root.locked ? 0.4 : 1.0
                Behavior on color { ColorAnimation { duration: 150 } }
                Behavior on border.color { ColorAnimation { duration: 150 } }

                Text {
                    anchors.centerIn: parent
                    text: "+ Add Source"
                    color: !root.locked && addMouse.containsMouse ? "#fff" : "#bbb"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                MouseArea {
                    id: addMouse
                    anchors.fill: parent
                    enabled: !root.locked
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: processPopup.open()
                }
            }

            Rectangle {
                id: removeBtn
                Layout.fillWidth: true
                height: 38
                radius: 8
                property bool canRemove: !root.locked && listView.currentIndex >= 0
                color: canRemove && removeMouse.containsMouse ? "#2a1414" : "#121212"
                border.color: canRemove && removeMouse.containsMouse ? "#ff5555" : "#1c1c1c"
                border.width: 1
                opacity: canRemove ? 1.0 : 0.4
                Behavior on color { ColorAnimation { duration: 150 } }
                Behavior on border.color { ColorAnimation { duration: 150 } }

                Text {
                    anchors.centerIn: parent
                    text: "- Remove"
                    color: removeBtn.canRemove && removeMouse.containsMouse ? "#ff5555" : "#bbb"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                MouseArea {
                    id: removeMouse
                    anchors.fill: parent
                    enabled: removeBtn.canRemove
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        var item = root.sourceModel[listView.currentIndex]
                        if (item) {
                            root.sourceRemoved(item.id)
                            listView.currentIndex = -1
                        }
                    }
                }
            }
        }
    }

    // Modern Process Selection Popup
    Popup {
        id: processPopup
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        width: Math.min(root.width * 0.95, 380)
        height: Math.min(root.height * 0.9, 460)
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 0

        property var allProcesses: []

        function refresh() {
            var raw = qmlBackend.getRunningProcesses()
            var seen = {}
            var deduped = []
            for (var i = 0; i < raw.length; i++) {
                var p = raw[i]
                if (!seen[p.name]) {
                    seen[p.name] = true
                    deduped.push(p)
                }
            }
            deduped.sort(function(a, b) {
                return a.name.toLowerCase().localeCompare(b.name.toLowerCase())
            })
            allProcesses = deduped
        }

        function acceptProcess() {
            var model = processList.model
            var idx = processList.currentIndex
            if (idx < 0 || typeof model !== 'object' || idx >= model.length) return
            var item = model[idx]
            var exeName = item.name
            var fullPath = item.fullPath
            var dotIdx = exeName.lastIndexOf(".")
            var displayName = dotIdx > 0 ? exeName.substring(0, dotIdx) : exeName
            root.sourceAdded(displayName, exeName, fullPath)
            processPopup.close()
        }

        background: Item {} // Transparent fallback to avoid style-specific customization warnings

        Rectangle {
            anchors.fill: parent
            color: "#0f0f0f"
            border.color: "#222"
            border.width: 1
            radius: 12

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Text {
                    text: "Select Process"
                    color: "white"
                    font.pixelSize: 16
                    font.weight: Font.Bold
                }

                // Style-safe Search/Filter Text Field
                Rectangle {
                    id: searchFieldContainer
                    Layout.fillWidth: true
                    height: 36
                    color: "#121212"
                    radius: 8
                    border.color: "#1c1c1c"
                    border.width: 1
                    Behavior on border.color { ColorAnimation { duration: 150 } }

                    TextInput {
                        id: searchField
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        verticalAlignment: TextInput.AlignVCenter
                        color: "white"
                        font.pixelSize: 12
                        selectByMouse: true

                        onActiveFocusChanged: {
                            searchFieldContainer.border.color = activeFocus ? "#BB86FC" : "#1c1c1c"
                        }

                        Text {
                            text: "Search running processes..."
                            color: "#4a4a4a"
                            font.pixelSize: 12
                            visible: !searchField.text && !searchField.activeFocus
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                // Filtered Process List
                ListView {
                    id: processList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    currentIndex: -1
                    spacing: 4

                    model: {
                        var raw = processPopup.allProcesses
                        if (!raw || raw.length === 0) return []
                        var q = searchField.text.trim().toLowerCase()
                        if (!q) return raw
                        var result = []
                        for (var i = 0; i < raw.length; i++) {
                            if (raw[i].name.toLowerCase().indexOf(q) >= 0 ||
                                raw[i].fullPath.toLowerCase().indexOf(q) >= 0) {
                                result.push(raw[i])
                            }
                        }
                        return result
                    }

                    delegate: Rectangle {
                        id: procItem
                        width: processList.width
                        height: 42
                        radius: 6
                        color: processList.currentIndex === index ? "#1c1c1c" : (procMouse.containsMouse ? "#121212" : "transparent")
                        border.color: processList.currentIndex === index ? "#BB86FC" : "transparent"
                        border.width: 1
                        Behavior on color { ColorAnimation { duration: 150 } }

                        MouseArea {
                            id: procMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: processList.currentIndex = index
                            onDoubleClicked: processPopup.acceptProcess()
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 10

                            // Executable Icon
                            Image {
                                source: modelData.fullPath ? "image://fileicon/" + encodeURIComponent(modelData.fullPath) : ""
                                width: 20
                                height: 20
                                fillMode: Image.PreserveAspectFit
                                Layout.preferredWidth: 20
                                Layout.preferredHeight: 20
                                opacity: modelData.fullPath ? 0.9 : 0.2
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 1
                                Text {
                                    text: modelData.name
                                    color: "white"
                                    font.pixelSize: 12
                                    font.weight: Font.Medium
                                }
                                Text {
                                    text: modelData.fullPath
                                    color: "#444"
                                    font.pixelSize: 9
                                    elide: Text.ElideLeft
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        width: 6
                        policy: ScrollBar.AsNeeded
                    }
                }

                // Popup Actions
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Rectangle {
                        id: popupCancel
                        Layout.fillWidth: true
                        height: 36
                        radius: 8
                        color: cancelMouse.containsMouse ? "#1c1c1c" : "#121212"
                        border.color: "#1c1c1c"
                        border.width: 1
                        Behavior on color { ColorAnimation { duration: 150 } }

                        Text {
                            anchors.centerIn: parent
                            text: "Cancel"
                            color: cancelMouse.containsMouse ? "#777" : "#444"
                            font.pixelSize: 12
                        }

                        MouseArea {
                            id: cancelMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: processPopup.close()
                        }
                    }

                    Rectangle {
                        id: popupAdd
                        Layout.fillWidth: true
                        height: 36
                        radius: 8
                        property bool canAdd: processList.currentIndex >= 0
                        color: canAdd && addProcMouse.containsMouse ? "#222" : "#121212"
                        border.color: canAdd && addProcMouse.containsMouse ? "#333" : "#1c1c1c"
                        border.width: 1
                        opacity: canAdd ? 1.0 : 0.4
                        Behavior on color { ColorAnimation { duration: 150 } }

                        Text {
                            anchors.centerIn: parent
                            text: "Add"
                            color: popupAdd.canAdd && addProcMouse.containsMouse ? "#fff" : "#bbb"
                            font.pixelSize: 12
                            font.weight: Font.Medium
                        }

                        MouseArea {
                            id: addProcMouse
                            anchors.fill: parent
                            enabled: popupAdd.canAdd
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: processPopup.acceptProcess()
                        }
                    }
                }
            }
        }

        onOpened: {
            refresh()
            searchField.text = ""
            processList.currentIndex = -1
            searchField.forceActiveFocus()
        }
    }
}
