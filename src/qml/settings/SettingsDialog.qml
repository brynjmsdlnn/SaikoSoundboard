import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

SaikoFramelessPopup {
    id: root
    width: 720
    height: 540

    signal changeBaseRequested()
    signal changeRecordingRequested()
    signal changeReplayRequested()
    signal resetRecordingRequested()
    signal resetReplayRequested()

    property string activeTab: "storage"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left Sidebar
        SettingsSidebar {
            activeTab: root.activeTab
            onTabSelected: function(tab) {
                root.activeTab = tab
            }
        }

        // Right Content Area
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            StorageSettingsPanel {
                id: storageContent
                visible: root.activeTab === "storage"
                onChangeBaseRequested: root.changeBaseRequested()
                onChangeRecordingRequested: root.changeRecordingRequested()
                onChangeReplayRequested: root.changeReplayRequested()
                onResetRecordingRequested: root.resetRecordingRequested()
                onResetReplayRequested: root.resetReplayRequested()
            }

            AudioSettingsPanel {
                id: audioContent
                visible: root.activeTab === "audio"
            }

            RoutingSettingsPanel {
                id: routingContent
                visible: root.activeTab === "routing"
            }

            NotificationSettingsPanel {
                id: notificationContent
                visible: root.activeTab === "notifications"
            }
        }
    }
}
