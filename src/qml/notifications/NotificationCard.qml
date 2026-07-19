import QtQuick 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0
import "helpers.js" as NotifHelpers

// ---------------------------------------------------------------------------
// NotificationCard — single notification card rendered inside the list
// ---------------------------------------------------------------------------
Rectangle {
    id: card

    // ── Incoming properties ────────────────────────────────────────────────
    property string cardTitle
    property string cardMessage
    property string cardIcon
    property double cardExpiryTime
    property int cardDurationMs
    property bool cardIsFadingOut
    property int cardPlayCount
    property bool cardStackDuration
    property string cardPlaybackMode
    property double cardCurrentTime

    property int cardResolvedWidth
    property int cardResolvedHeight
    property bool cardIsRightSide

    // ── Computed helpers ───────────────────────────────────────────────────
    readonly property string _resolvedIcon: NotifHelpers.resolveIcon(cardIcon, cardPlaybackMode)
    readonly property color _accentColor: NotifHelpers.resolveAccentColor(cardIcon, cardPlaybackMode, Theme)
    readonly property int _timeLeftMs: cardExpiryTime === 0
        ? (cardDurationMs > 0 ? cardDurationMs : Backend.notifications.durationMs)
        : Math.max(0, cardExpiryTime - cardCurrentTime)

    implicitWidth: cardResolvedWidth
    implicitHeight: cardResolvedHeight
    color: Theme.cardBackground
    radius: Theme.cardRadius
    border.color: Theme.borderDefault
    border.width: 1
    opacity: cardIsFadingOut ? 0.0 : 1.0

    // ── Slide animation ────────────────────────────────────────────────────
    property real _slideOffset: cardIsRightSide ? 100 : -100
    transform: Translate { x: card._slideOffset }

    Behavior on opacity {
        NumberAnimation { duration: 250; easing.type: Easing.OutQuad }
    }
    Behavior on _slideOffset {
        NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
    }

    Component.onCompleted: { _slideOffset = 0 }

    // ── Layout ─────────────────────────────────────────────────────────────
    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // Icon circle
        Rectangle {
            width: 32
            height: 32
            radius: 16
            color: Qt.rgba(card._accentColor.r, card._accentColor.g, card._accentColor.b, 0.1)
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32

            Image {
                source: "image://icons/" + card._resolvedIcon + "?color=" + NotifHelpers.encodeColor(card._accentColor)
                sourceSize: Qt.size(16, 16)
                anchors.centerIn: parent
            }
        }

        // Title + badges column
        ColumnLayout {
            spacing: 2
            Layout.fillWidth: true

            RowLayout {
                spacing: 6
                Layout.fillWidth: true

                Text {
                    text: card.cardTitle
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeHeading
                    font.weight: Font.Bold
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                // xN combo badge — shown when a slot has been triggered multiple times
                Rectangle {
                    id: comboBadge
                    visible: (card.cardStackDuration || card.cardPlaybackMode === "LayeredRingOut") && card.cardPlayCount > 1
                    implicitWidth: comboText.implicitWidth + 10
                    implicitHeight: 18
                    radius: 9
                    color: Qt.rgba(card._accentColor.r, card._accentColor.g, card._accentColor.b, 0.15)
                    border.color: card._accentColor
                    border.width: 1
                    Layout.alignment: Qt.AlignVCenter

                    Text {
                        id: comboText
                        anchors.centerIn: parent
                        text: {
                            var label = NotifHelpers.comboLabel(card.cardStackDuration, card.cardPlaybackMode);
                            return label !== "" ? "x" + card.cardPlayCount + " " + label : "x" + card.cardPlayCount;
                        }
                        color: card._accentColor
                        font.pixelSize: 10
                        font.weight: Font.Bold
                    }
                }

                // Exact duration text countdown (e.g. "2.4s")
                Text {
                    id: timerText
                    visible: !card.cardIsFadingOut
                    text: ((card._timeLeftMs / 1000).toFixed(1)) + "s"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    font.weight: Font.Normal
                    Layout.alignment: Qt.AlignVCenter
                }
            }

            // Subtitle / message line
            Text {
                text: card.cardMessage
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeNormal
                elide: Text.ElideRight
                Layout.fillWidth: true
                visible: text !== ""
            }
        }
    }

    // ── Accent sidebar indicator ───────────────────────────────────────────
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 3
        color: card._accentColor
        radius: Theme.borderRadius
    }

    // ── Smooth horizontal progress bar ─────────────────────────────────────
    Rectangle {
        id: progressBar
        anchors.left: parent.left
        anchors.leftMargin: 3
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 1
        height: 2
        color: card._accentColor
        opacity: 0.4
        visible: !card.cardIsFadingOut

        readonly property real _totalDur: card.cardDurationMs > 0 ? card.cardDurationMs : Backend.notifications.durationMs
        readonly property real _progressRatio: _totalDur > 0 ? Math.min(1.0, Math.max(0.0, card._timeLeftMs / _totalDur)) : 0

        width: Math.max(0, (parent.width - 4) * _progressRatio)
    }
}
