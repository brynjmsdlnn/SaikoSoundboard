import QtQuick 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

ColumnLayout {
    id: root

    property string title: ""
    property string subtitle: ""

    Layout.fillWidth: true
    spacing: 4

    Text {
        text: root.title
        color: Theme.textPrimary
        font.pixelSize: 20
        font.weight: Font.Bold
    }
    Text {
        text: root.subtitle
        color: Theme.textDim
        font.pixelSize: Theme.fontSizeNormal || 12
    }
}
