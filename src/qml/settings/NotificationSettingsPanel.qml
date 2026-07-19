import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

SettingsPanelWrapper {
    title: "Notifications"
    subtitle: "Configure toast alerts and overlay settings."

    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
            width: scroll.availableWidth
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: cardLayout.implicitHeight + 32
                color: Theme.cardBackground
                radius: Theme.cardRadius
                border.color: Theme.borderDefault
                border.width: 1

                ColumnLayout {
                    id: cardLayout
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    anchors.topMargin: 16
                    spacing: 20

                    // --- HEADER (Preserved) ---
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        SaikoSectionLabel {
                            text: "ALERT PREFERENCES"
                        }
                        Text {
                            text: "Show toast notifications when you trigger hotkeys or finish audio recordings. The overlay stays visible on top of other windows/games."
                            color: Theme.textDim
                            font.pixelSize: Theme.fontSizeNormal
                            wrapMode: Text.WordWrap
                            lineHeight: 1.15
                            Layout.fillWidth: true
                        }
                    }

                    // --- 1. CUSTOM INLINE SWITCH (Main Toggle) ---
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        Text {
                            Layout.fillWidth: true
                            text: "Enable Notifications"
                            font.pixelSize: Theme.fontSizeNormal
                            font.weight: Font.DemiBold
                            color: Theme.textPrimary
                        }

                        // Reusable Switch Component
                        SaikoSwitch {
                            id: enabledSwitch
                            checked: Backend.notifications.enabled
                            onCheckedChanged: {
                                Backend.notifications.enabled = checked;
                                Backend.settings.save();
                            }
                        }
                    }

                    // --- 2. FEATURE CARD WITH CUSTOM INLINE SWITCH ---
                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 72
                        visible: enabledSwitch.checked // Collapses when disabled
                        color: Theme.cardBackground
                        radius: Theme.cardRadius
                        border.color: overlaySwitch.checked ? Theme.accentPurple : Theme.borderDefault
                        border.width: overlaySwitch.checked ? 2 : 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    Layout.fillWidth: true
                                    text: "Overlay Mode (Always On Top)"
                                    color: Theme.textPrimary
                                    font.weight: Font.DemiBold
                                    font.pixelSize: Theme.fontSizeNormal
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: "Keeps notifications visible above games and other windows."
                                    color: Theme.textDim
                                    font.pixelSize: Theme.fontSizeSmall
                                    wrapMode: Text.WordWrap
                                }
                            }

                            // Reusable Switch Component (Overlay)
                            SaikoSwitch {
                                id: overlaySwitch
                                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                                checked: Backend.notifications.overlayEnabled
                                onCheckedChanged: {
                                    Backend.notifications.overlayEnabled = checked;
                                    Backend.settings.save();
                                }
                            }
                        }
                    }

                    // --- 3. CUSTOM INLINE REAL-TIME SLIDER ---
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        visible: enabledSwitch.checked // Collapses when disabled

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                Layout.fillWidth: true
                                text: "Notification Duration"
                                color: Theme.textSecondary
                                font.pixelSize: Theme.fontSizeNormal
                                font.weight: Font.DemiBold
                            }
                            Text {
                                // Cleaned up formatting: converted from ms to whole seconds without decimals
                                text: Math.round(durationSlider.value / 1000) + "s"
                                color: Theme.accentPurple
                                font.pixelSize: Theme.fontSizeNormal
                                font.weight: Font.Bold
                            }
                        }

                        // Inline Custom Slider Component
                        Slider {
                            id: durationSlider
                            Layout.fillWidth: true
                            from: 2000  // 2s minimum
                            to: 10000   // 10s maximum
                            stepSize: 1000 // 1s intervals
                            value: Backend.notifications.durationMs || 3000
                            snapMode: Slider.SnapAlways

                            live: true

                            background: Rectangle {
                                x: durationSlider.leftPadding
                                y: durationSlider.topPadding + durationSlider.availableHeight / 2 - height / 2
                                width: durationSlider.availableWidth
                                height: 4
                                radius: 2
                                color: Theme.borderDefault

                                Rectangle {
                                    width: durationSlider.visualPosition * parent.width
                                    height: parent.height
                                    color: Theme.accentPurple
                                    radius: 2
                                }
                            }

                            handle: Rectangle {
                                x: durationSlider.leftPadding + durationSlider.visualPosition * durationSlider.availableWidth - width / 2
                                y: durationSlider.topPadding + durationSlider.availableHeight / 2 - height / 2
                                width: 14
                                height: 14
                                radius: 7
                                color: durationSlider.pressed ? Theme.accentPurple : Theme.textPrimary
                                border.color: Theme.accentPurple
                                border.width: 1
                            }

                            onPressedChanged: {
                                // If pressed becomes false, it means the user just let go!
                                if (!pressed) {
                                    Backend.notifications.durationMs = value;
                                    Backend.settings.save();
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: "2s"
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSizeSmall
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            Text {
                                text: "10s"
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSizeSmall
                            }
                        }
                    }

                    // --- 4. SPATIAL GRID PICKER ---
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        visible: enabledSwitch.checked

                        Text {
                            text: "Screen Position"
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSizeNormal
                            font.weight: Font.DemiBold
                        }

                        Rectangle {
                            id: screenContainer
                            Layout.fillWidth: true
                            implicitHeight: 120
                            color: Qt.darker(Theme.cardBackground, 1.15)
                            radius: Theme.cardRadius
                            border.color: Theme.borderDefault
                            border.width: 1
                            clip: true

                            // Vertical Center Line
                            Rectangle {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                width: 1
                                color: Theme.borderDefault
                                opacity: 0.5
                            }

                            // Horizontal Center Line
                            Rectangle {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 1
                                color: Theme.borderDefault
                                opacity: 0.5
                            }

                            // The Mockup Component
                            Rectangle {
                                id: positionIndicator
                                color: Theme.accentPurple
                                border.color: "#FFFFFF"
                                border.width: 2

                                // Fixed base values to avoid math feedback loops
                                width: 42
                                height: 22
                                radius: 4

                                // Fallback initial positions
                                x: 16
                                y: 16

                                // Custom boolean flags to control the layout inside the container cleanly
                                property bool showContent: true

                                // This script block catches when your position changes and runs the animation timeline cleanly
                                function animateToPosition(newPos) {
                                    flightTimeline.stop();

                                    // Calculate static destination coordinates based on the future layout look
                                    var targetX = 16;
                                    var targetY = 16;

                                    if (newPos === "TopRight" || newPos === "BottomRight") {
                                        targetX = screenContainer.width - 42 - 16;
                                    }
                                    if (newPos === "BottomLeft" || newPos === "BottomRight") {
                                        targetY = screenContainer.height - 22 - 16;
                                    }

                                    // Inject targets directly into our sequential script properties
                                    shrinkAnimW.to = 14;
                                    shrinkAnimH.to = 14;
                                    shrinkAnimR.to = 7;

                                    // When moving, the target X/Y needs to offset by the dot size difference (14 vs 42)
                                    // so that it matches up visually with the expansion zone
                                    var travelX = (newPos === "TopRight" || newPos === "BottomRight") ? screenContainer.width - 14 - 16 : 16;
                                    var travelY = (newPos === "BottomLeft" || newPos === "BottomRight") ? screenContainer.height - 14 - 16 : 16;

                                    moveAnimX.to = travelX;
                                    moveAnimY.to = travelY;

                                    growAnimX.to = targetX;
                                    growAnimY.to = targetY;

                                    flightTimeline.start();
                                }

                                // Connection listener that intercepts backend updates
                                Connections {
                                    target: Backend.notifications
                                    function onPositionChanged() {
                                        positionIndicator.animateToPosition(Backend.notifications.position);
                                    }
                                }

                                // Component initialization safety fallback
                                Component.onCompleted: {
                                    var currentPos = Backend.notifications.position;
                                    if (currentPos === "TopRight" || currentPos === "BottomRight") {
                                        positionIndicator.x = screenContainer.width - 42 - 16;
                                    }
                                    if (currentPos === "BottomLeft" || currentPos === "BottomRight") {
                                        positionIndicator.y = screenContainer.height - 22 - 16;
                                    }
                                }

                                // Multi-Stage Sequential Flight Pipeline
                                SequentialAnimation {
                                    id: flightTimeline

                                    // Stage 1: Collapse into a dot & hide content lines
                                    PropertyAction {
                                        target: positionIndicator
                                        property: "showContent"
                                        value: false
                                    }
                                    ParallelAnimation {
                                        NumberAnimation {
                                            id: shrinkAnimW
                                            target: positionIndicator
                                            property: "width"
                                            duration: 100
                                            easing.type: Easing.OutQuad
                                        }
                                        NumberAnimation {
                                            id: shrinkAnimH
                                            target: positionIndicator
                                            property: "height"
                                            duration: 100
                                            easing.type: Easing.OutQuad
                                        }
                                        NumberAnimation {
                                            id: shrinkAnimR
                                            target: positionIndicator
                                            property: "radius"
                                            duration: 100
                                            easing.type: Easing.OutQuad
                                        }
                                    }

                                    // Stage 2: Fly across the card container frame
                                    ParallelAnimation {
                                        NumberAnimation {
                                            id: moveAnimX
                                            target: positionIndicator
                                            property: "x"
                                            duration: 220
                                            easing.type: Easing.OutCubic
                                        }
                                        NumberAnimation {
                                            id: moveAnimY
                                            target: positionIndicator
                                            property: "y"
                                            duration: 220
                                            easing.type: Easing.OutCubic
                                        }
                                    }

                                    // Stage 3: Expand outward back into notification card size
                                    ParallelAnimation {
                                        NumberAnimation {
                                            target: positionIndicator
                                            property: "width"
                                            to: 42
                                            duration: 120
                                            easing.type: Easing.OutQuad
                                        }
                                        NumberAnimation {
                                            target: positionIndicator
                                            property: "height"
                                            to: 22
                                            duration: 120
                                            easing.type: Easing.OutQuad
                                        }
                                        NumberAnimation {
                                            target: positionIndicator
                                            property: "radius"
                                            to: 4
                                            duration: 120
                                            easing.type: Easing.OutQuad
                                        }
                                        NumberAnimation {
                                            id: growAnimX
                                            target: positionIndicator
                                            property: "x"
                                            duration: 120
                                            easing.type: Easing.OutQuad
                                        }
                                        NumberAnimation {
                                            id: growAnimY
                                            target: positionIndicator
                                            property: "y"
                                            duration: 120
                                            easing.type: Easing.OutQuad
                                        }
                                    }
                                    PropertyAction {
                                        target: positionIndicator
                                        property: "showContent"
                                        value: true
                                    }
                                }

                                // Embedded Mock Layout elements inside the notification shape
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    spacing: 3
                                    opacity: positionIndicator.showContent ? 1.0 : 0.0
                                    Behavior on opacity {
                                        NumberAnimation {
                                            duration: 80
                                        }
                                    }

                                    Rectangle {
                                        width: 3
                                        height: 3
                                        radius: 1.5
                                        color: "#FFFFFF"
                                    }
                                    Column {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Rectangle {
                                            width: parent.width * 0.8
                                            height: 2
                                            color: "#FFFFFF"
                                            opacity: 0.8
                                        }
                                        Rectangle {
                                            width: parent.width * 0.5
                                            height: 2
                                            color: "#FFFFFF"
                                            opacity: 0.5
                                        }
                                    }
                                }
                            }

                            // 4 Transparent Interactive Click Zones
                            Grid {
                                anchors.fill: parent
                                columns: 2
                                rows: 2

                                Repeater {
                                    model: ["TopLeft", "TopRight", "BottomLeft", "BottomRight"]
                                    delegate: MouseArea {
                                        width: screenContainer.width / 2
                                        height: screenContainer.height / 2
                                        hoverEnabled: true

                                        Rectangle {
                                            anchors.fill: parent
                                            color: Theme.accentPurple
                                            opacity: parent.containsMouse ? 0.08 : 0
                                            Behavior on opacity {
                                                NumberAnimation {
                                                    duration: 150
                                                }
                                            }
                                        }

                                        onClicked: {
                                            Backend.notifications.position = modelData;
                                            Backend.settings.save();
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // --- 5. SEGMENTED CHIP GROUP ---
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        visible: enabledSwitch.checked

                        // Static text header
                        Text {
                            text: "Notification Size"
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSizeNormal
                            font.weight: Font.DemiBold
                        }

                        Rectangle {
                            id: segmentContainer
                            Layout.fillWidth: true
                            implicitHeight: 38
                            color: Qt.darker(Theme.cardBackground, 1.05)
                            radius: Theme.cardRadius
                            border.color: Theme.borderDefault
                            border.width: 1

                            readonly property var sizesList: ["ExtraSmall", "Small", "Medium", "Large", "ExtraLarge"]
                            readonly property int currentIndex: sizesList.indexOf(Backend.notifications.size)

                            // Smooth sliding selector block
                            Rectangle {
                                id: selectionHighlight
                                width: (segmentContainer.width - 6) / 5
                                height: segmentContainer.height - 6
                                y: 3
                                x: 3 + (segmentContainer.currentIndex * (segmentContainer.width / 5))
                                radius: Theme.cardRadius > 3 ? Theme.cardRadius - 2 : Theme.cardRadius
                                color: Theme.accentPurple

                                Behavior on x {
                                    NumberAnimation {
                                        duration: 180
                                        easing.type: Easing.OutQuad
                                    }
                                }
                            }

                            Row {
                                anchors.fill: parent

                                Repeater {
                                    model: [
                                        {
                                            label: "XS",
                                            value: "ExtraSmall",
                                            desc: "Extra Small"
                                        },
                                        {
                                            label: "S",
                                            value: "Small",
                                            desc: "Small"
                                        },
                                        {
                                            label: "M",
                                            value: "Medium",
                                            desc: "Medium"
                                        },
                                        {
                                            label: "L",
                                            value: "Large",
                                            desc: "Large"
                                        },
                                        {
                                            label: "XL",
                                            value: "ExtraLarge",
                                            desc: "Extra Large"
                                        }
                                    ]

                                    delegate: Item {
                                        width: segmentContainer.width / 5
                                        height: segmentContainer.height

                                        // Hover background tint card
                                        Rectangle {
                                            anchors.fill: parent
                                            anchors.margins: 3
                                            radius: Theme.cardRadius > 3 ? Theme.cardRadius - 2 : Theme.cardRadius
                                            color: Theme.textPrimary
                                            opacity: (btnMouse.containsMouse && Backend.notifications.size !== modelData.value) ? 0.06 : 0.0

                                            Behavior on opacity {
                                                NumberAnimation {
                                                    duration: 100
                                                }
                                            }
                                        }

                                        Text {
                                            id: buttonText
                                            anchors.centerIn: parent

                                            // Dynamically swap the text payload when hovered!
                                            text: btnMouse.containsMouse ? modelData.desc : modelData.label

                                            // Shrink the font size slightly if it's the long description so it doesn't clip
                                            font.pixelSize: btnMouse.containsMouse ? Theme.fontSizeNormal - 1 : Theme.fontSizeNormal
                                            font.weight: Backend.notifications.size === modelData.value ? Font.Bold : Font.Normal
                                            color: Backend.notifications.size === modelData.value ? "#FFFFFF" : Theme.textSecondary

                                            // 1. Smoothly fade text color changes
                                            Behavior on color {
                                                ColorAnimation {
                                                    duration: 120
                                                }
                                            }

                                            // 2. Cross-fade the text string swap so it doesn't pop aggressively
                                            Behavior on text {
                                                SequentialAnimation {
                                                    NumberAnimation {
                                                        target: buttonText
                                                        property: "opacity"
                                                        to: 0
                                                        duration: 50
                                                    }
                                                    PropertyAction {
                                                        target: buttonText
                                                        property: "text"
                                                    }
                                                    NumberAnimation {
                                                        target: buttonText
                                                        property: "opacity"
                                                        to: 1
                                                        duration: 80
                                                    }
                                                }
                                            }
                                        }

                                        MouseArea {
                                            id: btnMouse
                                            anchors.fill: parent
                                            hoverEnabled: true

                                            onClicked: {
                                                Backend.notifications.size = modelData.value;
                                                Backend.settings.save();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // --- PREVIEW BUTTON ---
                    SaikoButton {
                        id: previewBtn
                        Layout.fillWidth: true
                        Layout.topMargin: 8
                        visible: enabledSwitch.checked // Collapses when disabled
                        text: "Preview Notification"
                        filled: true
                        accentColor: Theme.accentPurple
                        onClicked: {
                            Backend.notifications.postNotification("Saiko Soundboard", "Preview — your settings look great!", "info");
                        }
                    }
                }
            }
        }
    }
}
