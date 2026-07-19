import QtQuick 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0
import "helpers.js" as NotifHelpers

// ---------------------------------------------------------------------------
// NotificationList — container managing model data, decay timers, and
// the "+N more" badge. Uses NotificationCard for each visible card.
// ---------------------------------------------------------------------------
Item {
    id: root
    clip: true

    // Whether this list is running in the global standalone overlay window (true)
    // or inside the main application window (false)
    property bool isOverlayMode: false

    readonly property int count: notificationModel.count
    property double currentTime: Date.now()

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

    // ── Auto-collapse after 4 seconds when expanded ────────────────────────
    Timer {
        id: collapseTimer
        interval: 4000
        repeat: false
        onTriggered: root.expanded = false
    }

    // ── Notification model ─────────────────────────────────────────────────
    ListModel {
        id: notificationModel
    }

    // ── Decay timer — lazily initialises expiry and removes expired cards ──
    Timer {
        id: decayTimer
        interval: 100
        repeat: true
        running: notificationModel.count > 0
        onTriggered: {
            root.currentTime = Date.now();
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

    // ── C++ backend connections ────────────────────────────────────────────
    Connections {
        target: Backend.notifications

        function onNotificationCollapsed(sourceId) {
            var now = Date.now();
            for (var i = 0; i < notificationModel.count; ++i) {
                var item = notificationModel.get(i);
                if (item.sourceId === sourceId && !item.isFadingOut) {
                    notificationModel.setProperty(i, "playCount", 0);
                    notificationModel.setProperty(i, "expiryTime", now);
                    notificationModel.setProperty(i, "isFadingOut", true);
                    return;
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

        function onNotificationPosted(title, message, icon, durationMs, sourceId, stackDuration, playbackMode) {
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

            // Cut-All clearance: if the incoming notification is LayeredCutAll,
            // immediately collapse all other active playback notifications
            if (playbackMode === "LayeredCutAll") {
                for (var ci = 0; ci < notificationModel.count; ++ci) {
                    var candidate = notificationModel.get(ci);
                    if (candidate.sourceId !== sourceId && !candidate.isFadingOut && candidate.icon === "play") {
                        notificationModel.setProperty(ci, "expiryTime", now);
                        notificationModel.setProperty(ci, "isFadingOut", true);
                    }
                }
            }

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
                            var currentDuration = item.durationMs || 0;
                            notificationModel.setProperty(i, "durationMs", currentDuration + durationMs);
                        } else {
                            notificationModel.setProperty(i, "expiryTime", now + durationMs);
                            notificationModel.setProperty(i, "durationMs", durationMs);
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
                "expiryTime": stackDuration ? (now + durationMs) : 0,
                "durationMs": durationMs,
                "isFadingOut": false,
                "createdTime": now,
                "sourceId": sourceId || "",
                "playCount": 1,
                "stackDuration": stackDuration,
                "playbackMode": playbackMode || ""
            });
        }
    }

    // ── Visual layout ──────────────────────────────────────────────────────
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
            delegate: NotificationCard {
                // Only cards within the viewport are visible; queued cards collapse to 0 height
                // When expanded, all cards become visible up to displayCount
                visible: index < root.displayCount
                Layout.fillWidth: true

                cardTitle: model.title
                cardMessage: model.message
                cardIcon: model.icon
                cardExpiryTime: model.expiryTime
                cardDurationMs: model.durationMs
                cardIsFadingOut: model.isFadingOut
                cardPlayCount: model.playCount
                cardStackDuration: model.stackDuration
                cardPlaybackMode: model.playbackMode
                cardCurrentTime: root.currentTime

                cardResolvedWidth: root.resolvedSize.width
                cardResolvedHeight: root.resolvedSize.height
                cardIsRightSide: root.isRightSide
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
            // Use the first visible card's accent color, or default to purple if none visible
            readonly property color badgeAccentColor: root.visibleCount > 0 && notificationModel.get(0)
                ? NotifHelpers.resolveAccentColor(notificationModel.get(0).icon, notificationModel.get(0).playbackMode, Theme)
                : Theme.accentPurple
            color: badgeMouse.containsMouse
                ? Qt.rgba(badge.badgeAccentColor.r, badge.badgeAccentColor.g, badge.badgeAccentColor.b, 0.15)
                : Qt.rgba(badge.badgeAccentColor.r, badge.badgeAccentColor.g, badge.badgeAccentColor.b, 0.08)
            border.color: badgeMouse.containsMouse ? badge.badgeAccentColor : Theme.borderDefault
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
                    source: "image://icons/layers?color=" + NotifHelpers.encodeColor(Theme.textSecondary)
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
