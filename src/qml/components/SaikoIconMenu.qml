import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Popup {
    id: menu

    // ── Public API ──────────────────────────────────────────────────────────
    property var model: []
    property int currentIndex: -1

    /// Emitted with the index of the item that was clicked
    signal activated(int index)

    /// Opens the menu positioned below and right-aligned to the given trigger item.
    /// `coordinateParent` should be the item that this popup is a child of in the QML
    /// hierarchy (typically the parent item where this Popup is declared).
    function openRelativeTo(triggerItem, coordinateParent) {
        var pos = triggerItem.mapToItem(coordinateParent, 0, triggerItem.height + 4);
        x = pos.x + triggerItem.width - width;
        y = pos.y;
        open();
    }

    // ── Layout ──────────────────────────────────────────────────────────────
    width: 220
    padding: 4
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 110 }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 80 }
    }

    background: Rectangle {
        color: "#171717"
        border.color: Theme.borderHover
        border.width: 1
        radius: Theme.cardRadius
    }

    contentItem: ColumnLayout {
        spacing: 2

        Repeater {
            model: menu.model

            delegate: Rectangle {
                id: itemDelegate
                required property string icon
                required property string label
                required property string modeColor
                required property int index

                Layout.fillWidth: true
                Layout.preferredHeight: 32
                radius: 6
                color: itemArea.containsMouse ? "#242424" : "transparent"

                Behavior on color {
                    ColorAnimation { duration: 100 }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 8

                    Image {
                        source: "image://icons/" + itemDelegate.icon
                                + "?color=%23"
                                + (itemArea.containsMouse
                                    ? "b0b0b0"
                                    : itemDelegate.modeColor.replace("#", ""))
                        sourceSize: Qt.size(14, 14)
                        Layout.alignment: Qt.AlignVCenter
                    }

                    Text {
                        text: itemDelegate.label
                        color: itemDelegate.index === menu.currentIndex
                            ? Theme.warning
                            : (itemArea.containsMouse ? Theme.textPrimary : Theme.textSecondary)
                        font.pixelSize: 12
                        font.weight: itemDelegate.index === menu.currentIndex ? Font.Bold : Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    // Active indicator dot
                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        visible: itemDelegate.index === menu.currentIndex
                        color: Theme.warning
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                MouseArea {
                    id: itemArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onClicked: {
                        menu.activated(itemDelegate.index)
                        menu.close()
                    }
                }
            }
        }
    }
}
