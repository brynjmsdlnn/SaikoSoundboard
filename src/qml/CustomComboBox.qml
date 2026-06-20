import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Templates 2.15 as T
import Saiko 1.0

T.ComboBox {
    id: combo
    textRole: "text"
    valueRole: "value"
    
    implicitWidth: 120
    implicitHeight: 36
    
    font.pixelSize: 12
    property int radius: 8

    contentItem: Text {
        leftPadding: combo.font.pixelSize >= 12 ? 12 : 6
        rightPadding: combo.font.pixelSize >= 12 ? 28 : 16
        text: combo.displayText
        font: combo.font
        color: Theme.textPrimary
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        color: combo.hovered ? "#1a1a1a" : Theme.inputBackground
        border.color: combo.visualFocus ? Theme.accentPurple : (combo.hovered ? Theme.borderHover : Theme.borderDefault)
        border.width: 1
        radius: combo.radius
        Behavior on color { ColorAnimation { duration: 150 } }
        Behavior on border.color { ColorAnimation { duration: 150 } }

        Text {
            text: "▼"
            anchors.right: parent.right
            anchors.rightMargin: combo.font.pixelSize >= 12 ? 12 : 6
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: combo.font.pixelSize >= 12 ? 8 : 6
            color: combo.hovered ? Theme.textPrimary : Theme.textDim
            Behavior on color { ColorAnimation { duration: 150 } }
        }
    }

    popup: T.Popup {
        y: combo.height + 4
        width: combo.width
        implicitHeight: Math.min(contentItem.implicitHeight + 8, 200)
        padding: 4

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: combo.popup.visible ? combo.delegateModel : null
            currentIndex: combo.highlightedIndex

            ScrollBar.vertical: ScrollBar {
                width: 6
                policy: ScrollBar.AsNeeded
            }
        }

        background: Rectangle {
            color: Theme.appBackground
            border.color: Theme.borderHover
            border.width: 1
            radius: combo.radius
        }
    }

    delegate: T.ItemDelegate {
        id: delegateItem
        width: combo.width - 8
        height: combo.font.pixelSize >= 12 ? 34 : 26
        implicitWidth: width
        implicitHeight: height
        anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
        hoverEnabled: true
        
        contentItem: Text {
            text: modelData.text || modelData
            color: delegateItem.highlighted || delegateItem.hovered ? Theme.textPrimary : Theme.textSecondary
            font: combo.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            leftPadding: combo.font.pixelSize >= 12 ? 10 : 6
        }
        
        background: Rectangle {
            color: delegateItem.highlighted || delegateItem.hovered ? Theme.borderDefault : "transparent"
            radius: combo.radius - 2
            Behavior on color { ColorAnimation { duration: 100 } }
        }
    }
}
