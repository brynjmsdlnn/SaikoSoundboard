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
    // Two-phase lifecycle: Playing (progress = audio) -> Decay (progress = settings duration)
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

                // Unified fade-out removal check:
                // If a card is fading out (manually collapsed or decayed),
                // remove it from the model 300ms after the fade started.
                if (item.isFadingOut) {
                    if (now > item.expiryTime + 300) {
                        notificationModel.remove(i);
                    }
                    continue;
                }

                if (i < root.displayCount) {
                    if (item.state === "Decay") {
                        // Decay phase: track configured decay duration
                        if (now > item.expiryTime) {
                            notificationModel.setProperty(i, "isFadingOut", true);
                            notificationModel.setProperty(i, "expiryTime", now);
                        }
                    } else {
                        // Playing phase: lazily initialize expiry when first visible
                        if (item.expiryTime === 0) {
                            var itemDur = item.durationMs > 0 ? item.durationMs : Backend.notifications.durationMs;
                            notificationModel.setProperty(i, "expiryTime", now + itemDur);
                        } else if (item.expiryTime > 0 && now > item.expiryTime) {
                            // Playing phase expired
                            if (!item.sourceId || item.sourceId === "") {
                                // Non-playback notifications (recording, slot-assigned, etc.):
                                // fade out immediately
                                notificationModel.setProperty(i, "isFadingOut", true);
                                notificationModel.setProperty(i, "expiryTime", now);
                            } else {
                                // Playback notifications: apply failsafe after 10s grace period
                                // to handle cases where playerStopped was not emitted
                                if (now > item.expiryTime + 10000) {
                                    notificationModel.setProperty(i, "state", "Decay");
                                    notificationModel.setProperty(i, "decayStartTime", now);
                                    notificationModel.setProperty(i, "expiryTime", now + Backend.notifications.durationMs);
                                }
                            }
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

        function onNotificationPlaybackStopped(sourceId, reason) {
            var now = Date.now();
            for (var i = 0; i < notificationModel.count; ++i) {
                var item = notificationModel.get(i);
                if (item.sourceId === sourceId && !item.isFadingOut) {
                    if (reason === "User" || reason === "Interrupted" || reason === "Error") {
                        // Collapse immediately
                        notificationModel.setProperty(i, "playCount", 0);
                        notificationModel.setProperty(i, "expiryTime", now);
                        notificationModel.setProperty(i, "isFadingOut", true);
                    } else if (reason === "Natural") {
                        // Transition to Decay state for ALL playback modes,
                        // including QueuedSequential (which now gets a Decay linger phase)
                        notificationModel.setProperty(i, "state", "Decay");
                        notificationModel.setProperty(i, "decayStartTime", now);
                        notificationModel.setProperty(i, "expiryTime", now + Backend.notifications.durationMs);
                    }
                    return;
                }
            }
        }

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

        function onNotificationActiveVoiceCountChanged(sourceId, count) {
            for (var i = 0; i < notificationModel.count; ++i) {
                var item = notificationModel.get(i);
                if (item.sourceId === sourceId && !item.isFadingOut) {
                    notificationModel.setProperty(i, "activeVoiceCount", count);
                    return;
                }
            }
        }

        function onNotificationPlaybackUpdated(sourceId, queueCount, remainingMs) {
            var now = Date.now();
            for (var i = 0; i < notificationModel.count; ++i) {
                var item = notificationModel.get(i);
                if (item.sourceId === sourceId && !item.isFadingOut) {
                    notificationModel.setProperty(i, "playCount", 1 + queueCount);

                    if (item.stackDuration && remainingMs > 0) {
                        var newExpiry = now + remainingMs;
                        var elapsed = now - item.createdTime;
                        var newDuration = remainingMs + elapsed;

                        notificationModel.setProperty(i, "expiryTime", newExpiry);
                        notificationModel.setProperty(i, "durationMs", newDuration);
                    }
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

            // Merge with an existing active card for this sourceId
            if (sourceId && sourceId !== "") {
                for (var i = 0; i < notificationModel.count; ++i) {
                    var item = notificationModel.get(i);
                    if (String(item.sourceId) === String(sourceId) && !item.isFadingOut) {
                        // Check if we are re-triggering from the Decay phase
                        var wasDecaying = (item.state === "Decay");

                        if (wasDecaying) {
                            // Reset to a brand new playback session
                            notificationModel.setProperty(i, "playCount", 1);
                            notificationModel.setProperty(i, "expiryTime", now + durationMs);
                            notificationModel.setProperty(i, "durationMs", durationMs);
                            // Reset createdTime for fresh session
                            notificationModel.setProperty(i, "createdTime", now);
                        } else {
                            // Already playing — accumulate or reset based on mode
                            if (!stackDuration) {
                                var currentCount = item.playCount || 1;
                                notificationModel.setProperty(i, "playCount", currentCount + 1);
                            }

                            if (stackDuration) {
                                // Do nothing: durationMs and expiryTime are managed solely by
                                // onNotificationQueueCountChanged to avoid double-addition bugs.
                            } else if (playbackMode === "LayeredCutAll") {
                                // LayeredCutAll: do not reset duration or createdTime
                            } else {
                                // Restart/Toggle: reset play window to start from 'now'
                                notificationModel.setProperty(i, "createdTime", now);
                                notificationModel.setProperty(i, "expiryTime", now + durationMs);
                                notificationModel.setProperty(i, "durationMs", durationMs);
                            }
                        }

                        // Reset card state to Playing on re-trigger (handles Decay->Playing transition)
                        notificationModel.setProperty(i, "state", "Playing");
                        notificationModel.setProperty(i, "decayStartTime", 0);
                        notificationModel.setProperty(i, "isFadingOut", false);
                        return;
                    }
                }
            }

            // No existing card found — append a new one
            notificationModel.append({
                "title": title,
                "message": message,
                "icon": icon,
                "state": "Playing",
                "decayStartTime": 0,
                "expiryTime": now + durationMs,
                "durationMs": durationMs,
                "isFadingOut": false,
                "createdTime": now,
                "sourceId": sourceId || "",
                "playCount": 1,
                "stackDuration": stackDuration,
                "playbackMode": playbackMode || "",
                "activeVoiceCount": 1
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
                cardState: model.state
                cardDecayStartTime: model.decayStartTime
                cardExpiryTime: model.expiryTime
                cardDurationMs: model.durationMs
                cardCreatedTime: model.createdTime
                cardIsFadingOut: model.isFadingOut
                cardPlayCount: model.playCount
                cardStackDuration: model.stackDuration
                cardPlaybackMode: model.playbackMode
                cardActiveVoiceCount: model.activeVoiceCount
                cardCurrentTime: root.currentTime
                cardSourceId: model.sourceId

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
