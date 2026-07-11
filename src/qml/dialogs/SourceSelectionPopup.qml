import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Popup {
    id: root
    property var sourceModel: null
    signal processSelected(string name, string executableName, string executablePath)
    signal deviceSelected(string name, string deviceName)

    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    width: Math.min(parent.width * 0.95, 380)
    height: Math.min(parent.height * 0.9, 460)
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    padding: 0

    property string activeTab: "processes"
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
            if (a.isProducingSound && !b.isProducingSound) return -1;
            if (!a.isProducingSound && b.isProducingSound) return 1;
            return a.name.toLowerCase().localeCompare(b.name.toLowerCase())
        })
        allProcesses = deduped
        refreshButton.text = "Refreshed!"
        refreshButton.accentColor = Theme.accentTeal
        buttonTextResetTimer.restart()
        listFadeAnim.restart()
    }

    function acceptProcess() {
        var model = processList.model
        var idx = processList.currentIndex
        if (idx < 0 || typeof model !== 'object' || idx >= model.length) return
        var item = model[idx]
        if (root.activeTab === "processes") {
            var exeName = item.name
            var fullPath = item.fullPath
            var dotIdx = exeName.lastIndexOf(".")
            var displayName = dotIdx > 0 ? exeName.substring(0, dotIdx) : exeName
            root.processSelected(displayName, exeName, fullPath)
        } else {
            var devDesc = item.description
            var cleanName = devDesc
            var parenIdx = cleanName.indexOf(" (")
            if (parenIdx > 0) {
                cleanName = cleanName.substring(0, parenIdx)
            }
            root.deviceSelected(cleanName, devDesc)
        }
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

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: root.activeTab === "processes" ? "Select Process" : "Select Audio Device"
                    color: Theme.textPrimary
                    font.pixelSize: 16
                    font.weight: Font.Bold
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                }

                SaikoButton {
                    id: refreshButton
                    text: "Refresh"
                    small: true
                    visible: root.activeTab === "processes"
                    enabled: !buttonTextResetTimer.running
                    Layout.alignment: Qt.AlignVCenter
                    onClicked: root.refresh()
                }
            }

            // --- Premium Segmented Tab Selector ---
            RowLayout {
                Layout.fillWidth: true
                spacing: 0
                
                Rectangle {
                    Layout.fillWidth: true
                    height: 32
                    color: Theme.inputBackground
                    radius: 6
                    
                    RowLayout {
                        anchors.fill: parent
                        spacing: 2
                        anchors.margins: 2
                        
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 4
                            color: root.activeTab === "processes" ? Theme.cardBackground : "transparent"
                            border.color: root.activeTab === "processes" ? Theme.borderDefault : "transparent"
                            border.width: 1
                            
                            Text {
                                anchors.centerIn: parent
                                text: "Processes"
                                color: root.activeTab === "processes" ? Theme.textPrimary : Theme.textDim
                                font.pixelSize: 12
                                font.weight: root.activeTab === "processes" ? Font.DemiBold : Font.Normal
                            }
                            
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    root.activeTab = "processes"
                                    processList.currentIndex = -1
                                }
                            }
                        }
                        
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 4
                            color: root.activeTab === "devices" ? Theme.cardBackground : "transparent"
                            border.color: root.activeTab === "devices" ? Theme.borderDefault : "transparent"
                            border.width: 1
                            
                            Text {
                                anchors.centerIn: parent
                                text: "Audio Devices"
                                color: root.activeTab === "devices" ? Theme.textPrimary : Theme.textDim
                                font.pixelSize: 12
                                font.weight: root.activeTab === "devices" ? Font.DemiBold : Font.Normal
                            }
                            
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    root.activeTab = "devices"
                                    processList.currentIndex = -1
                                }
                            }
                        }
                    }
                }
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
                        text: root.activeTab === "processes" ? "Search running processes..." : "Search audio devices..."
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
                    var q = searchField.text.trim().toLowerCase()
                    if (root.activeTab === "processes") {
                        var raw = root.allProcesses
                        if (!raw || raw.length === 0) return []
                        if (!q) return raw
                        var result = []
                        for (var i = 0; i < raw.length; i++) {
                            if (raw[i].name.toLowerCase().indexOf(q) >= 0 ||
                                raw[i].fullPath.toLowerCase().indexOf(q) >= 0) {
                                result.push(raw[i])
                            }
                        }
                        return result
                    } else {
                        var rawDevs = Backend.getAudioOutputDevices()
                        if (!rawDevs || rawDevs.length === 0) return []
                        var resultDevs = []
                        for (var d = 0; d < rawDevs.length; d++) {
                            var dev = rawDevs[d]
                            if (root.sourceModel && !root.sourceModel.hasDevice(dev.description)) {
                                if (!q || dev.description.toLowerCase().indexOf(q) >= 0) {
                                    resultDevs.push(dev)
                                }
                            }
                        }
                        return resultDevs
                    }
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
                            source: {
                                if (root.activeTab === "devices") {
                                    return "image://icons/volume-2?color=%23b0b0b0"
                                }
                                return modelData.fullPath ? "image://fileicon/" + encodeURIComponent(modelData.fullPath) : ""
                            }
                            width: 20
                            height: 20
                            fillMode: Image.PreserveAspectFit
                            Layout.preferredWidth: 20
                            Layout.preferredHeight: 20
                            opacity: (root.activeTab === "devices" || modelData.fullPath) ? 0.9 : 0.2
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1
                            Text {
                                text: root.activeTab === "devices" ? modelData.description : modelData.name
                                color: Theme.textPrimary
                                font.pixelSize: Theme.fontSizeNormal
                                font.weight: Font.Medium
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                            Text {
                                text: root.activeTab === "devices" ? (modelData.isVirtual ? "Virtual Audio Cable" : "Audio Playback Device") : modelData.fullPath
                                color: Theme.textDim
                                font.pixelSize: 9
                                elide: Text.ElideLeft
                                Layout.fillWidth: true
                            }
                        }

                        Row {
                            id: audioIndicator
                            visible: root.activeTab === "processes" && !!modelData.isProducingSound
                            spacing: 2
                            Layout.alignment: Qt.AlignVCenter
                            Layout.rightMargin: 6

                            Rectangle {
                                width: 3
                                height: 8
                                color: Theme.accentTeal
                                radius: 1.5
                                SequentialAnimation on height {
                                    loops: Animation.Infinite
                                    PropertyAnimation { to: 14; duration: 400; easing.type: Easing.InOutQuad }
                                    PropertyAnimation { to: 6; duration: 350; easing.type: Easing.InOutQuad }
                                    PropertyAnimation { to: 10; duration: 450; easing.type: Easing.InOutQuad }
                                }
                            }
                            Rectangle {
                                width: 3
                                height: 12
                                color: Theme.accentTeal
                                radius: 1.5
                                SequentialAnimation on height {
                                    loops: Animation.Infinite
                                    PropertyAnimation { to: 6; duration: 300; easing.type: Easing.InOutQuad }
                                    PropertyAnimation { to: 15; duration: 450; easing.type: Easing.InOutQuad }
                                    PropertyAnimation { to: 8; duration: 350; easing.type: Easing.InOutQuad }
                                }
                            }
                            Rectangle {
                                width: 3
                                height: 6
                                color: Theme.accentTeal
                                radius: 1.5
                                SequentialAnimation on height {
                                    loops: Animation.Infinite
                                    PropertyAnimation { to: 12; duration: 350; easing.type: Easing.InOutQuad }
                                    PropertyAnimation { to: 5; duration: 400; easing.type: Easing.InOutQuad }
                                    PropertyAnimation { to: 14; duration: 300; easing.type: Easing.InOutQuad }
                                }
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

                SaikoButton {
                    id: popupCancel
                    text: "Cancel"
                    Layout.fillWidth: true
                    onClicked: root.close()
                }

                SaikoButton {
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

    function updateAudioStatus() {
        if (root.activeTab !== "processes") return
        var activeExes = Backend.getProcessesProducingSound()
        var activeSet = {}
        var hasMissingExe = false
        
        for (var i = 0; i < activeExes.length; i++) {
            var exe = activeExes[i]
            activeSet[exe] = true
            
            var found = false
            for (var k = 0; k < allProcesses.length; k++) {
                if (allProcesses[k].name.toLowerCase() === exe) {
                    found = true
                    break;
                }
            }
            if (!found) {
                hasMissingExe = true
            }
        }

        if (hasMissingExe) {
            refresh()
            return
        }

        var changed = false
        var updatedList = []
        for (var j = 0; j < allProcesses.length; j++) {
            var proc = allProcesses[j]
            var nameLower = proc.name.toLowerCase()
            var nowProducing = !!activeSet[nameLower]
            if (proc.isProducingSound !== nowProducing) {
                proc.isProducingSound = nowProducing
                changed = true
            }
            updatedList.push(proc)
        }

        if (changed) {
            updatedList.sort(function(a, b) {
                if (a.isProducingSound && !b.isProducingSound) return -1;
                if (!a.isProducingSound && b.isProducingSound) return 1;
                return a.name.toLowerCase().localeCompare(b.name.toLowerCase())
            })
            allProcesses = updatedList
        }
    }

    Timer {
        id: audioUpdateTimer
        interval: 1000
        repeat: true
        running: false
        onTriggered: root.updateAudioStatus()
    }

    onOpened: {
        activeTab = "processes"
        refresh()
        searchField.text = ""
        processList.currentIndex = -1
        searchField.forceActiveFocus()
        audioUpdateTimer.start()
    }

    NumberAnimation {
        id: listFadeAnim
        target: processList
        property: "opacity"
        from: 0.4
        to: 1.0
        duration: 200
        easing.type: Easing.OutQuad
    }

    Timer {
        id: buttonTextResetTimer
        interval: 1000
        repeat: false
        onTriggered: {
            refreshButton.text = "Refresh"
            refreshButton.accentColor = Theme.accentPurple
        }
    }

    onClosed: {
        audioUpdateTimer.stop()
        buttonTextResetTimer.stop()
        refreshButton.text = "Refresh"
        refreshButton.accentColor = Theme.accentPurple
    }
}
