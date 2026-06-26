import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

ColumnLayout {
    id: root

    property int outputRouting: 0
    property bool isLocked: false
    property int slotIndex: -1
    property var slotModel: null

    Layout.fillWidth: true
    spacing: 4

    SectionLabel {
        text: "OUTPUT ROUTING"
    }

    SaikoComboBox {
        Layout.fillWidth: true
        implicitHeight: 34
        enabled: !root.isLocked
        model: [
            { text: "Broadcast & monitor", value: 0 },
            { text: "Broadcast only",      value: 1 },
            { text: "Monitor only",        value: 2 }
        ]
        textRole: "text"
        valueRole: "value"
        currentIndex: root.outputRouting
        onActivated: {
            if (root.slotIndex >= 0)
                root.slotModel.setRouting(root.slotIndex, currentValue)
        }
    }
}
