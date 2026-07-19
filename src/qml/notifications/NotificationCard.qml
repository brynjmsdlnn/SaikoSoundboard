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
    property string cardState
    property double cardDecayStartTime
    property double cardExpiryTime
    property int cardDurationMs
    property double cardCreatedTime
    property bool cardIsFadingOut
    property int cardPlayCount
    property bool cardStackDuration
    property string cardPlaybackMode
    property double cardCurrentTime
    property int cardActiveVoiceCount: 1
    property string cardSourceId


    readonly property bool _showVoiceIndicators: !cardIsFadingOut && cardState === "Playing" && cardActiveVoiceCount > 1

    property int cardResolvedWidth
    property int cardResolvedHeight
    property bool cardIsRightSide

    // ── Computed helpers ───────────────────────────────────────────────────
    readonly property string _resolvedIcon: NotifHelpers.resolveIcon(cardIcon, cardPlaybackMode)
    readonly property color _accentColor: NotifHelpers.resolveAccentColor(cardIcon, cardPlaybackMode, Theme)
    readonly property bool isDecayPhase: cardState === "Decay"
    readonly property bool isPlaybackPlaying: cardSourceId !== "" && cardState === "Playing"

    // Direct expiryTime subtraction naturally tracks active queue durations
    readonly property int _timeLeftMs: Math.max(0, cardExpiryTime - cardCurrentTime)

    // Total duration for progress bar: audio duration in Playing, settings duration in Decay
    readonly property real _totalDur: isDecayPhase
        ? Backend.notifications.durationMs
        : (cardDurationMs > 0 ? cardDurationMs : Backend.notifications.durationMs)

    implicitWidth: cardResolvedWidth
    implicitHeight: cardResolvedHeight
    color: isDecayPhase ? Qt.rgba(Theme.cardBackground.r, Theme.cardBackground.g, Theme.cardBackground.b, 0.85) : Theme.cardBackground
    radius: Theme.cardRadius
    border.color: isDecayPhase ? Qt.rgba(card._accentColor.r, card._accentColor.g, card._accentColor.b, 0.3) : Theme.borderDefault
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

        // Icon circle with adaptive breathing rings
        Item {
            width: 32
            height: 32
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32

            // Glowing ring 1 — 2-3 voices
            Rectangle {
                anchors.centerIn: parent
                width: parent.width + 8
                height: parent.height + 8
                radius: width / 2
                color: Qt.rgba(card._accentColor.r, card._accentColor.g, card._accentColor.b, 0.15)
                visible: cardActiveVoiceCount >= 2 && cardState === "Playing"

                ScaleAnimator on scale {
                    running: cardActiveVoiceCount >= 2 && !card.cardIsFadingOut
                    loops: Animation.Infinite
                    from: 0.95; to: 1.1; duration: 1200; easing.type: Easing.InOutQuad
                }
            }

            // Glowing ring 2 — 4+ voices
            Rectangle {
                anchors.centerIn: parent
                width: parent.width + 16
                height: parent.height + 16
                radius: width / 2
                color: Qt.rgba(card._accentColor.r, card._accentColor.g, card._accentColor.b, 0.08)
                visible: cardActiveVoiceCount >= 4 && cardState === "Playing"

                ScaleAnimator on scale {
                    running: cardActiveVoiceCount >= 4 && !card.cardIsFadingOut
                    loops: Animation.Infinite
                    from: 0.9; to: 1.15; duration: 1600; easing.type: Easing.InOutQuad
                }
            }

            // Icon background circle
            Rectangle {
                anchors.fill: parent
                radius: 16
                color: Qt.rgba(card._accentColor.r, card._accentColor.g, card._accentColor.b, 0.1)

                Image {
                    source: "image://icons/" + card._resolvedIcon + "?color=" + NotifHelpers.encodeColor(card._accentColor)
                    sourceSize: Qt.size(16, 16)
                    anchors.centerIn: parent
                }
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

                // LED Voice Indicators — active voice dots
                Row {
                    spacing: 4
                    Layout.alignment: Qt.AlignVCenter
                    visible: card._showVoiceIndicators

                    Repeater {
                        model: cardActiveVoiceCount
                        delegate: Rectangle {
                            width: index === 0 ? 6 : 5
                            height: index === 0 ? 6 : 5
                            radius: width / 2
                            color: card._accentColor
                            opacity: index === 0 ? 1.0 : 0.6

                            SequentialAnimation on opacity {
                                running: index > 0 && !card.cardIsFadingOut
                                loops: Animation.Infinite
                                NumberAnimation { from: 0.3; to: 0.8; duration: 800; easing.type: Easing.InOutSine }
                                NumberAnimation { from: 0.8; to: 0.3; duration: 800; easing.type: Easing.InOutSine }
                            }
                        }
                    }
                }

                // xN combo badge — shown when a slot has been triggered multiple times (queued sequential only)
                Rectangle {
                    id: comboBadge
                    visible: card.cardStackDuration && card.cardPlayCount > 1
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
                        text: "x" + card.cardPlayCount
                        color: card._accentColor
                        font.pixelSize: 10
                        font.weight: Font.Bold
                    }
                }

                // Exact duration text countdown (e.g. "2.4s") — shown only during active playback and before expiry
                Text {
                    id: timerText
                    visible: card.isPlaybackPlaying && !card.cardIsFadingOut && card._timeLeftMs > 0
                    text: ((card._timeLeftMs / 1000).toFixed(1)) + "s"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    font.weight: Font.Normal
                    Layout.alignment: Qt.AlignVCenter
                }
            }

            // Subtitle / message line — shows "Finished" during decay phase
            Text {
                text: card.isDecayPhase ? "Finished" : card.cardMessage
                color: card.isDecayPhase ? Qt.rgba(card._accentColor.r, card._accentColor.g, card._accentColor.b, 0.6) : Theme.textSecondary
                font.pixelSize: Theme.fontSizeNormal
                font.weight: card.isDecayPhase ? Font.Medium : Font.Normal
                elide: Text.ElideRight
                Layout.fillWidth: true
                visible: true
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

    // ── Smooth horizontal progress bar — shown in all states (playing, decay, non-playback) ──
    Rectangle {
        id: progressBar
        anchors.left: parent.left
        anchors.leftMargin: 3
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 1
        height: 2
        color: card.isDecayPhase ? Qt.rgba(card._accentColor.r, card._accentColor.g, card._accentColor.b, 0.25) : card._accentColor
        opacity: 0.5
        visible: !card.cardIsFadingOut

        readonly property real _progressRatio: card._totalDur > 0 ? Math.min(1.0, Math.max(0.0, card._timeLeftMs / card._totalDur)) : 0

        width: Math.max(0, (parent.width - 4) * _progressRatio)
    }
}
