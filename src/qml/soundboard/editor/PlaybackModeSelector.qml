import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

ColumnLayout {
    id: root

    property int playbackMode: 0
    property bool isLocked: false
    property int slotIndex: -1
    property var slotModel: null

    Layout.fillWidth: true
    spacing: 4

    SectionLabel {
        text: "PLAYBACK MODE"
    }

    SaikoComboBox {
        Layout.fillWidth: true
        implicitHeight: 34
        enabled: !root.isLocked
        model: [
            { text: "Default (Global setting)",          value: 0 },
            { text: "Restart (Retrigger)",                value: 1 },
            { text: "Toggle Play / Stop",                 value: 2 },
            { text: "Queued Replay (Sequential)",         value: 3 },
            { text: "Layered Play (Cut All on Stop)",     value: 4 },
            { text: "Layered Play (Let Ring Out)",        value: 5 }
        ]
        textRole: "text"
        valueRole: "value"
        currentIndex: root.playbackMode
        onActivated: {
            if (root.slotIndex >= 0)
                root.slotModel.setPlaybackMode(root.slotIndex, currentValue)
        }
    }
}
