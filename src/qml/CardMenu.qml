import QtQuick
import QtQuick.Templates 2.15 as T
import Saiko 1.0

T.Menu {
    id: menu
    width: 168
    margins: 0
    implicitHeight: contentItem.contentHeight + 10

    contentItem: ListView {
        implicitHeight: contentHeight
        model: menu.contentModel
        interactive: false
        spacing: 2
    }

    background: Rectangle {
        implicitWidth: 168
        color: "#171717"
        border.color: Theme.borderHover
        border.width: 1
        radius: Theme.cardRadius
    }

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 110 }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 80 }
    }
}
