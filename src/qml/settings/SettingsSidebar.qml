import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Saiko 1.0

Rectangle {
    id: root

    property string activeTab: "storage"

    signal tabSelected(string tab)

    Layout.preferredWidth: 180
    Layout.fillHeight: true
    color: Theme.recessedBackground
    radius: Theme.cardRadius || 8

    // Cover to flatten the right-side rounded corners of the sidebar
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 16
        color: Theme.recessedBackground
    }

    // Vertical border line separating sidebar and content area
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: Theme.borderDefault
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 8

        Text {
            text: "SETTINGS"
            color: Theme.textDim
            font.pixelSize: 10
            font.weight: Font.Bold
            Layout.fillWidth: true
            Layout.bottomMargin: 8
        }

        // Storage Tab Button
        SidebarTabButton {
            id: storageTabBtn
            iconSource: "image://icons/folder?color=" + (root.activeTab === "storage" ? Theme.accentPurple : Theme.textSecondary)
            label: "Storage"
            isActive: root.activeTab === "storage"
            onClicked: root.tabSelected("storage")
        }

        // Audio Tab Button
        SidebarTabButton {
            id: audioTabBtn
            iconSource: "image://icons/audio-lines?color=" + (root.activeTab === "audio" ? Theme.accentPurple : Theme.textSecondary)
            label: "Audio Quality"
            isActive: root.activeTab === "audio"
            onClicked: root.tabSelected("audio")
        }

        // Routing Tab Button
        SidebarTabButton {
            id: routingTabBtn
            iconSource: "image://icons/route?color=" + (root.activeTab === "routing" ? Theme.accentPurple : Theme.textSecondary)
            label: "Audio Routing"
            isActive: root.activeTab === "routing"
            onClicked: root.tabSelected("routing")
        }

        // Notifications Tab Button
        SidebarTabButton {
            id: notificationsTabBtn
            iconSource: "image://icons/info?color=" + (root.activeTab === "notifications" ? Theme.accentPurple : Theme.textSecondary)
            label: "Notifications"
            isActive: root.activeTab === "notifications"
            onClicked: root.tabSelected("notifications")
        }

        Item { Layout.fillHeight: true }
    }

    // ── Internal tab button component ──────────────────────────────────────
    component SidebarTabButton: Rectangle {
        property string iconSource: ""
        property string label: ""
        property bool isActive: false

        signal clicked()

        Layout.fillWidth: true
        height: 36
        radius: 6
        color: isActive ? Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.1) : "transparent"
        border.color: isActive ? Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.2) : "transparent"
        border.width: 1

        Behavior on color {
            ColorAnimation { duration: Theme.animDuration }
        }
        Behavior on border.color {
            ColorAnimation { duration: Theme.animDuration }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 10

            Image {
                source: parent.parent.iconSource
                sourceSize: Qt.size(16, 16)
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                text: parent.parent.label
                color: parent.parent.isActive ? Theme.textPrimary : Theme.textSecondary
                font.pixelSize: 12
                font.weight: parent.parent.isActive ? Font.DemiBold : Font.Normal
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onEntered: {
                if (!parent.isActive) {
                    parent.color = Qt.rgba(255, 255, 255, 0.05)
                }
            }
            onExited: {
                if (!parent.isActive) {
                    parent.color = "transparent"
                }
            }
            onClicked: parent.clicked()
        }
    }
}
