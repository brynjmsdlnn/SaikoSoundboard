import QtQuick 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

Item {
    id: root
    clip: true

    // Whether this list is running in the global standalone overlay window (true)
    // or inside the main application window (false)
    property bool isOverlayMode: false

    readonly property int count: notificationModel.count

    // Preset size lookup mapping
    readonly property var sizeMap: {
        "ExtraSmall": { width: 240, height: 48 },
        "Small":      { width: 280, height: 56 },
        "Medium":     { width: 320, height: 64 },
        "Large":      { width: 360, height: 72 },
        "ExtraLarge": { width: 400, height: 80 }
    }

    property string notificationSize: Backend.notifications.size
    property string notificationPosition: Backend.notifications.position

    readonly property var resolvedSize: sizeMap[notificationSize] || sizeMap["Medium"]
    readonly property bool isRightSide: notificationPosition === "TopRight" || notificationPosition === "BottomRight"
    readonly property bool isBottomSide: notificationPosition === "BottomLeft" || notificationPosition === "BottomRight"

    // Max list height: cap at 400px or half the available screen height for the overlay
    readonly property int maxHeight: Math.min(400, Screen.desktopAvailableHeight * 0.5)
    // Dynamic capacity: how many cards fit within maxHeight (with 8px spacing + 8px top/bottom padding)
    readonly property int maxCount: Math.max(1, Math.floor((maxHeight - 8) / (resolvedSize.height + 8)))

    // Number of cards currently visible in the viewport (the rest are queued)
    // When expanded, all cards are shown up to maxHeight; otherwise capped at maxCount
    property bool expanded: false
    readonly property int displayCount: expanded ? notificationModel.count : maxCount
    readonly property int visibleCount: Math.min(notificationModel.count, displayCount)
    readonly property bool hasBadge: !expanded && notificationModel.count > maxCount
    // Badge height (collapses to 0 when not shown or when expanded)
    readonly property int badgeHeight: hasBadge ? 36 : 0

    width: resolvedSize.width
    implicitHeight: 400
    height: visibleCount > 0 ? (visibleCount * (resolvedSize.height + 8) + badgeHeight + 8) : 0

    // Auto-collapse after 4 seconds when expanded
    Timer {
        id: collapseTimer
        interval: 4000
        repeat: false
        onTriggered: root.expanded = false
    }

    ListModel {
        id: notificationModel
    }

    function encodeColor(c) {
        return String(c).replace("#", "%23");
    }

    Timer {
        id: decayTimer
        interval: 100
        repeat: true
        running: notificationModel.count > 0
        onTriggered: {
            var now = Date.now();
            for (var i = notificationModel.count - 1; i >= 0; --i) {
                var item = notificationModel.get(i);
                if (i < root.displayCount) {
                    // QueuedSequential cards: no timer-based expiry; collapse is driven
                    // entirely by notificationCollapsed from the player state
                    if (item.stackDuration) {
                        if (item.isFadingOut && now > item.expiryTime + 300) {
                            notificationModel.remove(i);
                        }
                        continue;
                    }
                    // Viewport items: lazily initialize expiry when first visible
                    if (item.expiryTime === 0) {
                        var itemDur = item.durationMs > 0 ? item.durationMs : Backend.notifications.durationMs;
                        notificationModel.setProperty(i, "expiryTime", now + itemDur);
                    } else if (now > item.expiryTime + 300) {
                        notificationModel.remove(i);
                    } else if (now > item.expiryTime) {
                        if (!item.isFadingOut) {
                            notificationModel.setProperty(i, "isFadingOut", true);
                        }
                    }
                } else {
                    // Queued items: reset expiry to 0 so they don't decay while hidden
                    if (item.expiryTime !== 0) {
                        notificationModel.setProperty(i, "expiryTime", 0);
                    }
                }
            }
        }
    }

    Connections {
        target: Backend.notifications

        function onNotificationCollapsed(sourceId) {
            // Decrement playCount; only collapse when count reaches 0
            var now = Date.now();
            for (var i = 0; i < notificationModel.count; ++i) {
                var item = notificationModel.get(i);
                if (item.sourceId === sourceId && !item.isFadingOut) {
                    var currentCount = item.playCount || 1;
                    var newCount = currentCount - 1;
                    if (newCount <= 0) {
                        notificationModel.setProperty(i, "playCount", 0);
                        notificationModel.setProperty(i, "expiryTime", now);
                        notificationModel.setProperty(i, "isFadingOut", true);
                    } else {
                        notificationModel.setProperty(i, "playCount", newCount);
                    }
                    return; // Handle exactly one layer stop event
                }
            }
        }

        function onNotificationQueueCountChanged(sourceId, queueCount) {
            for (var i = 0; i < notificationModel.count; ++i) {
                var item = notificationModel.get(i);
                if (item.sourceId === sourceId && !item.isFadingOut) {
                    notificationModel.setProperty(i, "playCount", 1 + queueCount);
                    return;
                }
            }
        }

        function onNotificationPosted(title, message, icon, durationMs, sourceId, stackDuration) {
            // Check global enabled setting
            if (!Backend.notifications.enabled) {
                return;
            }

            // Show in standalone overlay window ONLY if overlay mode is enabled.
            // Show inside the app window ONLY if overlay mode is disabled.
            if (Backend.notifications.overlayEnabled !== root.isOverlayMode) {
                return;
            }

            // Extend the expanded state when new notifications arrive
            if (root.expanded) {
                collapseTimer.restart();
            }

            var now = Date.now();

            // Merge with an existing active card for this sourceId
            if (sourceId && sourceId !== "") {
                for (var i = 0; i < notificationModel.count; ++i) {
                    var item = notificationModel.get(i);
                    if (String(item.sourceId) === String(sourceId) && !item.isFadingOut) {
                        // Found existing card — increment playCount
                        if (!stackDuration) {
                            var currentCount = item.playCount || 1;
                            notificationModel.setProperty(i, "playCount", currentCount + 1);
                        }

                        // stackDuration: accumulate expiry for queued sequential mode
                        // resetDuration: restart expiry from now for layered/overlapping modes
                        if (stackDuration) {
                            var currentExpiry = item.expiryTime || now;
                            notificationModel.setProperty(i, "expiryTime", Math.max(currentExpiry, now) + durationMs);
                        } else {
                            notificationModel.setProperty(i, "expiryTime", now + durationMs);
                        }
                        return;
                    }
                }
            }

            // No existing card found — append a new one
            notificationModel.append({
                "title": title,
                "message": message,
                "icon": icon,
                "expiryTime": 0,
                "durationMs": durationMs,
                "isFadingOut": false,
                "createdTime": now,
                "sourceId": sourceId || "",
                "playCount": 1,
                "stackDuration": stackDuration
            });
        }
    }

    ColumnLayout {
        id: listLayout
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 8

        states: [
            State {
                name: "top"
                AnchorChanges {
                    target: listLayout
                    anchors.top: listLayout.parent.top
                    anchors.bottom: undefined
                }
            },
            State {
                name: "bottom"
                AnchorChanges {
                    target: listLayout
                    anchors.bottom: listLayout.parent.bottom
                    anchors.top: undefined
                }
            }
        ]
        state: root.isBottomSide ? "bottom" : "top"

        Repeater {
            model: notificationModel
            delegate: Rectangle {
                id: card
                // Only cards within the viewport are visible; queued cards collapse to 0 height
                // When expanded, all cards become visible up to displayCount
                visible: index < root.displayCount

                property int cardPlayCount: playCount

                Layout.preferredWidth: parent.width
                Layout.preferredHeight: root.resolvedSize.height
                color: Theme.cardBackground
                radius: Theme.cardRadius
                border.color: Theme.borderDefault
                border.width: 1
                opacity: isFadingOut ? 0.0 : 1.0

                // Slide offset: starts at ±100, animates to 0 on creation
                // Right-side positions slide from right (+100), left-side from left (-100)
                property real slideOffset: root.isRightSide ? 100 : -100

                transform: Translate {
                    x: card.slideOffset
                }

                Behavior on opacity {
                    NumberAnimation { duration: 250; easing.type: Easing.OutQuad }
                }

                Behavior on slideOffset {
                    NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                }

                Component.onCompleted: {
                    slideOffset = 0;
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Rectangle {
                        width: 32
                        height: 32
                        radius: 16
                        color: Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.1)
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32

                        Image {
                            source: "image://icons/" + (icon || "info") + "?color=" + encodeColor(Theme.accentPurple)
                            sourceSize: Qt.size(16, 16)
                            anchors.centerIn: parent
                        }
                    }

                    ColumnLayout {
                        spacing: 2
                        Layout.fillWidth: true

                        RowLayout {
                            spacing: 6
                            Layout.fillWidth: true

                            Text {
                                text: title
                                color: Theme.textPrimary
                                font.pixelSize: Theme.fontSizeHeading
                                font.weight: Font.Bold
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            // xN combo badge — shown when a slot has been triggered multiple times
                            // In a Repeater delegate, model roles (like playCount) are exposed
                            // as direct properties in the delegate scope — use bare name, not model.
                            Rectangle {
                                id: comboBadge
                                visible: card.cardPlayCount > 1
                                implicitWidth: comboText.implicitWidth + 10
                                implicitHeight: 18
                                radius: 9
                                color: Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.15)
                                border.color: Theme.accentPurple
                                border.width: 1
                                Layout.alignment: Qt.AlignVCenter

                                Text {
                                    id: comboText
                                    anchors.centerIn: parent
                                    text: "x" + card.cardPlayCount
                                    color: Theme.accentPurple
                                    font.pixelSize: 10
                                    font.weight: Font.Bold
                                }
                            }
                        }

                        Text {
                            text: message
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSizeNormal
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            visible: text !== ""
                        }
                    }
                }

                // Accent sidebar indicator
                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 3
                    color: Theme.accentPurple
                    radius: Theme.borderRadius
                }
            }
        }

        // "+N more" badge — shown at the end of the stack when queued cards exist
        Rectangle {
            id: badge
            visible: root.hasBadge
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: 32
            Layout.topMargin: 4
            radius: 16
            color: badgeMouse.containsMouse
                ? Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.15)
                : Qt.rgba(Theme.accentPurple.r, Theme.accentPurple.g, Theme.accentPurple.b, 0.08)
            border.color: badgeMouse.containsMouse ? Theme.accentPurple : Theme.borderDefault
            border.width: 1

            Behavior on color {
                ColorAnimation { duration: 150 }
            }
            Behavior on border.color {
                ColorAnimation { duration: 150 }
            }

            MouseArea {
                id: badgeMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.expanded = true;
                    collapseTimer.start();
                }
            }

            Row {
                anchors.centerIn: parent
                spacing: 8

                // Stacking layers icon
                Image {
                    source: "image://icons/layers?color=" + encodeColor(Theme.textSecondary)
                    sourceSize: Qt.size(16, 16)
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: "+" + (root.count - root.maxCount) + " more"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    font.weight: Font.DemiBold
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
