import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Saiko 1.0

ColumnLayout {
    id: root

    property string slotId: ""
    property string filePath: ""
    property string slotName: ""
    property bool isTemp: false
    property bool isLocked: false
    property bool fileExists: true

    Layout.fillWidth: true
    spacing: 8

    SaikoSectionLabel {
        text: "AUDIO SOURCE"
    }

    Rectangle {
        id: sourceBox
        Layout.fillWidth: true
        implicitHeight: 36
        radius: 6
        color: assignBtnArea.containsMouse ? Theme.recessedBackground : Theme.inputBackground

        border.color: {
            if (!root.fileExists && root.filePath !== "") {
                return Theme.accentRed;
            }
            return isTemp ? (assignBtnArea.containsMouse ? Theme.warning : Theme.warningDark) : (assignBtnArea.containsMouse ? Theme.accentPurple : Theme.borderDefault);
        }
        border.width: 1

        Behavior on border.color { ColorAnimation { duration: 150 } }
        Behavior on color { ColorAnimation { duration: 150 } }

        MouseArea {
            id: assignBtnArea
            anchors.fill: parent
            cursorShape: root.isLocked ? Qt.ArrowCursor : Qt.PointingHandCursor
            hoverEnabled: !root.isLocked
            onClicked: { if (!root.isLocked) assignMenu.open() }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 8
            spacing: 8

            Image {
                source: (!root.fileExists && root.filePath !== "") ? "image://icons/triangle-alert?color=" + encodeURIComponent(Theme.accentRed) : "image://icons/music?color=%23888888"
                smooth: true
                sourceSize: Qt.size(16, 16)
            }

            Rectangle {
                visible: isTemp
                implicitWidth: tempText.implicitWidth + 12
                implicitHeight: 20
                radius: 4
                color: "#33250a"
                border.color: Theme.warningDark
                border.width: 1

                Text {
                    id: tempText
                    anchors.centerIn: parent
                    text: "TEMP"
                    color: Theme.warning
                    font.pixelSize: 10
                    font.bold: true
                }
            }

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                text: root.filePath ? root.filePath.split("/").pop() : "No file assigned"
                color: {
                    if (!root.fileExists && root.filePath !== "") return Theme.accentRed;
                    return root.filePath ? Theme.textPrimary : Theme.textSecondary;
                }
                font.pixelSize: 13
                font.italic: !root.filePath
            }

            Rectangle {
                visible: root.isLocked
                implicitWidth: lockText.implicitWidth + 12
                implicitHeight: 20
                radius: 4
                color: "#2a1a08"
                border.color: Theme.warningDark
                border.width: 1

                Text {
                    id: lockText
                    anchors.centerIn: parent
                    text: "LOCKED"
                    color: Theme.warning
                    font.pixelSize: 10
                    font.bold: true
                }
            }

            Image {
                source: "image://icons/chevron-down?color=%23888888"
                sourceSize: Qt.size(16, 16)
                visible: !root.isLocked
            }

            Rectangle {
                visible: isTemp && !root.isLocked
                implicitWidth: 26
                implicitHeight: 26
                radius: 4
                color: permanentBtnArea.containsMouse ? "#33250a" : "#1f1606"
                border.color: permanentBtnArea.containsMouse ? Theme.warning : "transparent"
                border.width: 1

                Image {
                    anchors.centerIn: parent
                    source: "image://icons/save?color=%23d99a3d"
                    sourceSize: Qt.size(14, 14)
                }

                ToolTip {
                    visible: permanentBtnArea.containsMouse
                    text: "Save to disk (Make permanent)"
                    delay: 400
                }

                MouseArea {
                    id: permanentBtnArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onClicked: saveFileNameDialog.open()
                }
            }
        }

        SaikoTooltip {
            text: "File does not exist on disk:\n" + root.filePath
            hovered: assignBtnArea.containsMouse && !root.fileExists && root.filePath !== ""
            direction: "top"
            z: 999
        }

        SaikoMenu {
            id: assignMenu
            y: parent.height + 4
            x: sourceBox.width - width

            SaikoMenuItem {
                text: "From file..."
                onClicked: assignFileDialog.open()
            }
            SaikoMenuItem {
                text: "From replay buffer"
                onClicked: {
                    if (root.slotId)
                        Backend.actions.dispatchAssignReplay(root.slotId)
                }
            }
        }
    }

    FileDialog {
        id: assignFileDialog
        title: "Select audio file"
        nameFilters: ["Audio files (*.wav *.mp3 *.ogg)"]
        onAccepted: {
            if (selectedFile && root.slotId)
                Backend.soundboard.assignAudioFile(root.slotId, selectedFile)
        }
    }

    SaikoDialog {
        id: saveFileNameDialog
        title: "Save Permanent File"
        text: "Enter a file name:"
        confirmText: "Save"
        confirmColor: Theme.accentPurple
        width: 300

        TextField {
            id: saveNameField
            Layout.fillWidth: true
            text: saveFileNameDialog.stripExtension(root.filePath ? root.filePath.split("/").pop() : root.slotName)
            selectByMouse: true
            color: Theme.textPrimary
            font.pixelSize: 14
            background: Rectangle {
                color: Theme.recessedBackground
                border.color: saveNameField.activeFocus ? Theme.accentPurple : Theme.borderDefault
                radius: 6
            }
            onAccepted: saveFileNameDialog.saveAction()
        }

        onAccepted: saveFileNameDialog.saveAction()

        function stripExtension(name) {
            var idx = name.lastIndexOf(".")
            return idx > 0 ? name.substring(0, idx) : name
        }

        function saveAction() {
            if (root.slotId && root.slotId !== "") {
                var finalName = saveNameField.text.trim() || root.slotName
                Backend.actions.dispatchMakePermanent(root.slotId, finalName)
            }
            saveFileNameDialog.close()
        }

        onOpened: {
            var raw = root.filePath ? root.filePath.split("/").pop() : root.slotName
            saveNameField.text = stripExtension(raw)
            saveNameField.forceActiveFocus()
            saveNameField.selectAll()
        }
    }
}
