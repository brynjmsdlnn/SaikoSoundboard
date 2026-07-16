import QtQuick 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

ColumnLayout {
    id: root

    property string title: ""
    property string subtitle: ""

    default property alias panelContent: contentArea.data

    anchors.fill: parent
    anchors.margins: 24
    spacing: 16

    SettingsPanelHeader {
        title: root.title
        subtitle: root.subtitle
    }

    Item {
        id: contentArea
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
}
