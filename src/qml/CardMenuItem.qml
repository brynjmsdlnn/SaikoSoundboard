import QtQuick
import QtQuick.Templates 2.15 as T
import Saiko 1.0

T.MenuItem {
    id: menuItem
    implicitWidth: 160
    implicitHeight: 30
    padding: 4

    contentItem: Text {
        text: menuItem.text
        color: menuItem.hovered ? Theme.textPrimary : Theme.textSecondary
        font.pixelSize: Theme.fontSizeSmall
        leftPadding: 8
        verticalAlignment: Text.AlignVCenter
        Behavior on color { ColorAnimation { duration: 100 } }
    }

    background: Rectangle {
        color: menuItem.hovered ? "#242424" : "transparent"
        radius: Theme.borderRadius
        Behavior on color { ColorAnimation { duration: 100 } }
    }
}
