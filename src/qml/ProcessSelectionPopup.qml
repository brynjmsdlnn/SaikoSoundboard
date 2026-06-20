import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Popup {
    id: root
    property var sourceModel: null
    signal processSelected(string name, string executableName, string executablePath)

    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    width: Math.min(parent.width * 0.95, 380)
    height: Math.min(parent.height * 0.9, 460)
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    padding: 0

    property var allProcesses: []

    function refresh() {
        var raw = Backend.getRunningProcesses()
        var seen = {}
        var deduped = []
        for (var i = 0; i < raw.length; i++) {
            var p = raw[i]
            if (!seen[p.name] && !root.sourceModel.hasExecutable(p.name)) {
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
        root.processSelected(displayName, exeName, fullPath)
        root.close()
    }

    background: Item {}

    Rectangle {
        anchors.fill: parent
        color: Theme.appBackground
        border.color: Theme.borderHover
        border.width: 1
        radius: 12

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Text {
                text: "Select Process"
                color: Theme.textPrimary
                font.pixelSize: 16
                font.weight: Font.Bold
            }

            Rectangle {
                id: searchFieldContainer
                Layout.fillWidth: true
                height: 36
                color: Theme.inputBackground
                radius: Theme.cardRadius
                border.color: Theme.borderDefault
                border.width: 1
                Behavior on border.color { ColorAnimation { duration: Theme.animDuration } }

                TextInput {
                    id: searchField
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    verticalAlignment: TextInput.AlignVCenter
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeNormal
                    selectByMouse: true

                    onActiveFocusChanged: {
                        searchFieldContainer.border.color = activeFocus ? Theme.accentPurple : Theme.borderDefault
                    }

                    Text {
                        text: "Search running processes..."
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSizeNormal
                        visible: !searchField.text && !searchField.activeFocus
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            ListView {
                id: processList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                currentIndex: -1
                spacing: 4

                model: {
                    var raw = root.allProcesses
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
                    radius: Theme.borderRadius
                    color: processList.currentIndex === index ? "#1c1c1c" : (procMouse.containsMouse ? Theme.inputBackground : "transparent")
                    border.color: processList.currentIndex === index ? Theme.accentPurple : "transparent"
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: Theme.animDuration } }

                    MouseArea {
                        id: procMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: processList.currentIndex = index
                        onDoubleClicked: root.acceptProcess()
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 10

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
                                color: Theme.textPrimary
                                font.pixelSize: Theme.fontSizeNormal
                                font.weight: Font.Medium
                            }
                            Text {
                                text: modelData.fullPath
                                color: Theme.textDim
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

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                ThemedButton {
                    id: popupCancel
                    text: "Cancel"
                    Layout.fillWidth: true
                    onClicked: root.close()
                }

                ThemedButton {
                    id: popupAdd
                    text: "Add"
                    Layout.fillWidth: true
                    accentColor: Theme.accentPurple
                    enabled: processList.currentIndex >= 0
                    onClicked: root.acceptProcess()
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
