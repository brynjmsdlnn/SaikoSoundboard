import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

SaikoFramelessPopup {
    id: root
    width: 440
    height: 480

    property string filePath: ""
    property string selectedSlotId: ""
    property string selectedSlotName: ""

    // Column width constants for perfect alignment
    readonly property int colIndexWidth: 24
    readonly property int colHotkeyWidth: 80

    signal accepted()
    signal rejected()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        ColumnLayout {
            spacing: 4
            Text {
                text: "Assign to Slot"
                color: Theme.textPrimary
                font.pixelSize: 20
                font.weight: Font.Bold
            }
            Text {
                text: "Select a soundboard slot to assign this audio file."
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeNormal
            }
        }

        Rectangle {
            id: listBorder
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.cardBackground
            border.color: Theme.borderDefault
            radius: Theme.cardRadius
            clip: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 0

                // ── Header row ─────────────────────────────────────
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 32
                    color: "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 10

                        Item {
                            Layout.preferredWidth: root.colIndexWidth
                        }

                        SaikoSectionLabel {
                            text: "SLOT / FILE"
                            Layout.fillWidth: true
                        }

                        SaikoSectionLabel {
                            text: "HOTKEY"
                            Layout.preferredWidth: root.colHotkeyWidth
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 1
                    color: Theme.borderDefault
                }

                // ── Slot list ──────────────────────────────────────
                ListView {
                    id: slotList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: SlotModel
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true

                    delegate: Rectangle {
                        id: delegateRoot
                        width: slotList.width
                        height: 52
                        radius: 0
                        color: {
                            if (root.selectedSlotId === model.slotId) {
                                return Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.12);
                            }
                            return delegateMouseArea.containsMouse ? Theme.inputBackground : "transparent";
                        }

                        property bool isHovered: delegateMouseArea.containsMouse
                        property bool isSelected: root.selectedSlotId === model.slotId
                        property bool _nameMarqueeActive: (isSelected || isHovered)
                        property bool _fileMarqueeActive: (isSelected || isHovered)

                        Behavior on color {
                            ColorAnimation {
                                duration: Theme.animDuration
                            }
                        }

                        // Left accent bar for selected item
                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: 3
                            radius: 1.5
                            color: delegateRoot.isSelected ? Theme.accentPurple : "transparent"
                            visible: delegateRoot.isSelected

                            Behavior on color {
                                ColorAnimation {
                                    duration: Theme.animDuration
                                }
                            }
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 10

                            // ── Index column ─────────────────────────
                            Text {
                                text: (index + 1).toString()
                                color: delegateRoot.isSelected ? Theme.accentPurple : Theme.textDim
                                font.bold: true
                                font.pixelSize: Theme.fontSizeNormal
                                Layout.preferredWidth: root.colIndexWidth
                                horizontalAlignment: Text.AlignLeft
                            }

                            // ── Slot Name + File Name stacked column ──
                        ColumnLayout {
                            id: infoColumn
                            Layout.fillWidth: true
                            spacing: 2
                            Layout.alignment: Qt.AlignVCenter

                            // ── Slot name row with marquee ────────────
                            Item {
                                id: slotNameClip
                                Layout.fillWidth: true
                                implicitHeight: slotNameText.implicitHeight
                                clip: true

                                Text {
                                    id: slotNameText
                                    text: model.slotName ? model.slotName : "Empty Slot"
                                    color: delegateRoot.isSelected ? Theme.textPrimary : (model.slotName ? Theme.textPrimary : Theme.textDim)
                                    font.weight: Font.Medium
                                    font.pixelSize: Theme.fontSizeNormal
                                    width: delegateRoot._nameMarqueeActive ? implicitWidth : slotNameClip.width
                                    elide: delegateRoot._nameMarqueeActive ? Text.ElideNone : Text.ElideRight
                                    clip: false

                                    SequentialAnimation on x {
                                        running: delegateRoot._nameMarqueeActive && slotNameText.implicitWidth > slotNameClip.width && !!model.slotName
                                        loops: Animation.Infinite
                                        PauseAnimation { duration: 1500 }
                                        NumberAnimation {
                                            from: 0
                                            to: slotNameClip.width - slotNameText.implicitWidth
                                            duration: Math.max(2000, (slotNameText.implicitWidth - slotNameClip.width) * 25)
                                            easing.type: Easing.Linear
                                        }
                                        PauseAnimation { duration: 2000 }
                                        PropertyAction { value: 0 }
                                        PauseAnimation { duration: 500 }
                                        onRunningChanged: {
                                            if (!running) {
                                                slotNameText.x = 0;
                                            }
                                        }
                                    }
                                }
                            }

                            // ── File name row with badges + marquee ────
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 4
                                implicitHeight: 16

                                // ── Temp badge ────────────────────────────
                                Rectangle {
                                    id: tempBadge
                                    implicitWidth: tempLabel.implicitWidth + 10
                                    implicitHeight: 16
                                    radius: 3
                                    color: "#33250a"
                                    border.color: Theme.warningDark
                                    border.width: 1
                                    visible: model.isTemporary && !!model.filePath
                                    Layout.alignment: Qt.AlignVCenter

                                    Text {
                                        id: tempLabel
                                        anchors.centerIn: parent
                                        text: "TEMP"
                                        color: Theme.warning
                                        font.pixelSize: 8
                                        font.bold: true
                                    }
                                }

                                // ── Missing file warning ───────────────────
                                Image {
                                    id: missingIcon
                                    source: "image://icons/triangle-alert?color=" + encodeURIComponent(Theme.accentRed)
                                    sourceSize: Qt.size(12, 12)
                                    visible: !model.fileExists && model.filePath !== ""
                                    Layout.alignment: Qt.AlignVCenter
                                }

                                // ── File name text with marquee ────────────
                                Item {
                                    id: fileNameClip
                                    Layout.fillWidth: true
                                    implicitHeight: fileNameText.implicitHeight
                                    clip: true
                                    Layout.alignment: Qt.AlignVCenter

                                    Text {
                                        id: fileNameText
                                        text: model.filePath ? model.filePath.split("/").pop() : "No file assigned"
                                        color: delegateRoot.isSelected ? Theme.textSecondary : (model.filePath ? (model.fileExists ? Theme.textDim : Theme.accentRed) : Theme.textDim)
                                        font.pixelSize: Theme.fontSizeSmall
                                        width: delegateRoot._fileMarqueeActive ? implicitWidth : fileNameClip.width
                                        elide: delegateRoot._fileMarqueeActive ? Text.ElideNone : Text.ElideRight
                                        clip: false

                                        SequentialAnimation on x {
                                            running: delegateRoot._fileMarqueeActive && fileNameText.implicitWidth > fileNameClip.width && !!model.filePath
                                            loops: Animation.Infinite
                                            PauseAnimation { duration: 1500 }
                                            NumberAnimation {
                                                from: 0
                                                to: fileNameClip.width - fileNameText.implicitWidth
                                                duration: Math.max(2000, (fileNameText.implicitWidth - fileNameClip.width) * 25)
                                                easing.type: Easing.Linear
                                            }
                                            PauseAnimation { duration: 2000 }
                                            PropertyAction { value: 0 }
                                            PauseAnimation { duration: 500 }
                                            onRunningChanged: {
                                                if (!running) {
                                                    fileNameText.x = 0;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                            // ── Hotkey column ───────────────────────
                            Item {
                                Layout.preferredWidth: root.colHotkeyWidth
                                Layout.fillHeight: true
                                clip: true

                                Rectangle {
                                    id: hotkeyBadge
                                    anchors.centerIn: parent
                                    width: hotkeyText.implicitWidth + 12
                                    height: 20
                                    radius: 4
                                    color: "#2a2a2a"
                                    border.color: "#3a3a3a"
                                    border.width: 1
                                    visible: !!model.playHotkey

                                    Text {
                                        id: hotkeyText
                                        anchors.centerIn: parent
                                        text: model.playHotkey
                                        font.pixelSize: 11
                                        font.weight: Font.DemiBold
                                        color: "#e0e0e0"
                                    }
                                }
                            }
                        }

                        // Bottom divider
                        Rectangle {
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 1
                            color: Theme.borderDefault
                            opacity: 0.5
                        }

                        MouseArea {
                            id: delegateMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.selectedSlotId = model.slotId;
                                root.selectedSlotName = model.slotName ? model.slotName : "Slot " + (index + 1);
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            SaikoButton {
                text: "Cancel"
                Layout.fillWidth: true
                onClicked: {
                    root.rejected();
                    root.close();
                }
            }

            SaikoButton {
                text: "Assign to Slot"
                Layout.fillWidth: true
                accentColor: Theme.accentPurple
                enabled: root.selectedSlotId !== ""
                onClicked: {
                    if (root.selectedSlotId !== "") {
                        Backend.soundboard.assignAudioFile(root.selectedSlotId, root.filePath);
                        root.accepted();
                        root.close();
                    }
                }
            }
        }
    }
}
