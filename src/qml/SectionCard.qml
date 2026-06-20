import QtQuick
import QtQuick.Layouts
import Saiko 1.0

Rectangle {
    id: section
    default property alias content: sectionLayout.children
    property string heading: ""

    Layout.fillWidth: true
    Layout.preferredHeight: sectionLayout.implicitHeight + 28
    color: Theme.cardBackground
    radius: Theme.cardRadius
    border.color: Theme.borderDefault
    border.width: 1

    ColumnLayout {
        id: sectionLayout
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        Text {
            visible: section.heading !== ""
            text: section.heading
            color: Theme.textDim
            font.pixelSize: Theme.fontSizeSmall
            font.letterSpacing: 1.2
            font.weight: Font.Bold
        }
    }
}
