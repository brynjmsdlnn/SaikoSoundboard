import QtQuick
import QtQuick.Layouts
import Saiko 1.0

// A single hotkey slot: shows the current binding, and turns into a
// "press any key" capture UI (with cancel / clear / error / timeout
// handling) when isRecording is set to true.
Rectangle {
    id: card

    // ------------------------------------------------------------------
    // Public API (set by the parent)
    // ------------------------------------------------------------------
    property string title: ""
    property string currentKey: ""
    property bool isRecording: false
    property color accentColor: "white"
    property bool hasUnsavedChanges: false

    signal clicked()
    signal keyCaptured(string sequence)
    signal captureFailed()

    // ------------------------------------------------------------------
    // Tuning constants — change timings/thresholds here, not in the logic
    // below.
    // ------------------------------------------------------------------
    readonly property int captureTimeoutSeconds: 5
    readonly property int criticalCountdownThreshold: 2   // countdown <= this => "urgent" styling
    readonly property int hintDelayMs: 2000                // delay before help text starts cycling
    readonly property int carouselIntervalMs: 1500         // help text cycle speed
    readonly property int errorDisplayMs: 1000             // how long "UNSUPPORTED" stays up
    readonly property int countdownTickMs: 1000
    readonly property int pulseNormalMs: 500
    readonly property int pulseCriticalMs: 180
    readonly property int colorAnimMs: 180
    readonly property int textFadeMs: 550
    readonly property int errorFlashInMs: 120
    readonly property int errorFlashOutMs: 350

    readonly property color bgDefault: "#111111"
    readonly property color bgHover: "#141414"
    readonly property color bgRecording: "#161616"

    // ------------------------------------------------------------------
    // Internal state
    // ------------------------------------------------------------------
    property bool isHovered: false
    property int countdown: captureTimeoutSeconds
    property int pulseDuration: pulseNormalMs
    property bool showError: false
    property string errorMessage: ""
    property bool isGracePeriod: false

    // Cycles through: "press any key" -> "esc to cancel" -> "backspace to clear" -> repeat
    readonly property var helpCycleStates: [
        card.statePRESS_ANY_KEY,
        card.stateESC_TO_CANCEL,
        card.stateBACKSPACE_TO_CLEAR
    ]
    readonly property int statePRESS_ANY_KEY: 0
    readonly property int stateESC_TO_CANCEL: 1
    readonly property int stateBACKSPACE_TO_CLEAR: 2
    property int helpCycleIndex: card.statePRESS_ANY_KEY

    readonly property bool isCountdownCritical: card.countdown <= card.criticalCountdownThreshold

    // ------------------------------------------------------------------
    // Color logic
    //
    // Every colored element on this card follows the same priority order:
    //   error > grace-period (turns urgent once countdown is critical)
    //         > recording > idle
    // `idleColor` is whatever that element should look like when none of
    // the special states apply, so only the "idle" branch differs between
    // elements (e.g. the border also reacts to hover).
    // ------------------------------------------------------------------
    function priorityColor(idleColor) {
        if (card.showError) return Theme.errorDefault
        if (card.isGracePeriod) return card.isCountdownCritical ? Theme.errorDefault : card.accentColor
        if (card.isRecording) return card.accentColor
        return idleColor
    }

    // ------------------------------------------------------------------
    // Recording lifecycle
    // ------------------------------------------------------------------
    onIsRecordingChanged: {
        if (card.isRecording) {
            resetRecordingState()
            hiddenHintTimer.start()
        } else {
            stopAllTimers()
        }
    }

    function resetRecordingState() {
        card.countdown = card.captureTimeoutSeconds
        card.pulseDuration = card.pulseNormalMs
        card.showError = false
        card.errorMessage = ""
        card.isGracePeriod = false
        card.helpCycleIndex = card.statePRESS_ANY_KEY
        stopAllTimers()
    }

    function stopAllTimers() {
        hiddenHintTimer.stop()
        carouselTimer.stop()
        countdownTimer.stop()
        errorTimer.stop()
    }

    function handleCancelAction() {
        resetRecordingState()
        card.captureFailed()
    }

    function handleClearAction() {
        resetRecordingState()
        card.keyCaptured("")
    }

    function showErrorWithMessage(msg) {
        stopAllTimers()
        card.errorMessage = msg
        card.showError = true
        card.isGracePeriod = false
        card.countdown = card.captureTimeoutSeconds
        sharpErrorPulse.restart()
        errorTimer.restart()
    }

    // ------------------------------------------------------------------
    // Key-sequence handling
    // ------------------------------------------------------------------
    function isModifier(key) {
        return key === Qt.Key_Control || key === Qt.Key_Shift ||
               key === Qt.Key_Alt || key === Qt.Key_Meta
    }

    function isNumpadDigit(event) {
        // Qt reports numpad digits with the same key codes as the top-row
        // digits (Qt.Key_0..Qt.Key_9); KeypadModifier is what tells them apart.
        return (event.modifiers & Qt.KeypadModifier) &&
               event.key >= Qt.Key_0 && event.key <= Qt.Key_9
    }

    function keyName(key) {
        if (key === Qt.Key_Space) return "SPACE"
        if (key === Qt.Key_Return) return "ENTER"
        if (key === Qt.Key_Tab) return "TAB"
        if (key >= Qt.Key_0 && key <= Qt.Key_9) return String.fromCharCode(key)
        if (key >= Qt.Key_A && key <= Qt.Key_Z) return String.fromCharCode(key).toUpperCase()
        return ""
    }

    function getSequence(event) {
        var keyStr = card.keyName(event.key)
        if (keyStr === "") return ""

        // Treat numpad digits as distinct hotkeys from their top-row counterparts.
        if (card.isNumpadDigit(event)) keyStr = "NUM" + keyStr

        var parts = []
        if (event.modifiers & Qt.ControlModifier) parts.push("CTRL")
        if (event.modifiers & Qt.ShiftModifier) parts.push("SHIFT")
        if (event.modifiers & Qt.AltModifier) parts.push("ALT")
        parts.push(keyStr)
        return parts.join("+")
    }

    // Text shown in the small hint line while recording, cycling every
    // carouselIntervalMs once hintDelayMs has passed.
    function helpText() {
        switch (card.helpCycleIndex) {
            case card.stateESC_TO_CANCEL: return "ESC TO CANCEL"
            case card.stateBACKSPACE_TO_CLEAR: return "BACKSPACE TO CLEAR"
            default: return "PRESS ANY KEY"
        }
    }

    // ------------------------------------------------------------------
    // Visual root
    // ------------------------------------------------------------------
    Layout.fillWidth: true
    Layout.fillHeight: true
    radius: 12
    color: isRecording ? card.bgRecording : (isHovered ? card.bgHover : card.bgDefault)

    border.color: card.priorityColor(card.isHovered ? Theme.borderHover : (card.hasUnsavedChanges ? Theme.warning : Theme.borderDefault))
    border.width: 1

    Behavior on color { ColorAnimation { duration: card.colorAnimMs } }
    Behavior on border.color { ColorAnimation { duration: card.colorAnimMs } }

    // ------------------------------------------------------------------
    // Timers
    // ------------------------------------------------------------------

    // Initial delay before the help-text carousel kicks in, so the hint
    // doesn't flash up the instant recording starts.
    Timer {
        id: hiddenHintTimer
        interval: card.hintDelayMs
        repeat: false
        onTriggered: {
            if (card.isRecording && !card.showError && !card.isGracePeriod) {
                card.helpCycleIndex = card.stateESC_TO_CANCEL
                carouselTimer.start()
            }
        }
    }

    // Cycles the help text: esc-to-cancel -> backspace-to-clear -> press-any-key -> repeat.
    Timer {
        id: carouselTimer
        interval: card.carouselIntervalMs
        repeat: true
        onTriggered: {
            // esc-to-cancel -> backspace-to-clear -> press-any-key -> repeat.
            // The carousel is only ever started sitting on stateESC_TO_CANCEL,
            // so a plain wraparound increment reproduces that exact order.
            card.helpCycleIndex = (card.helpCycleIndex + 1) % card.helpCycleStates.length
        }
    }

    // Shows the "UNSUPPORTED" error message, then hands off to the countdown.
    Timer {
        id: errorTimer
        interval: card.errorDisplayMs
        onTriggered: {
            card.showError = false
            card.errorMessage = ""
            card.isGracePeriod = true
            countdownTimer.start()
        }
    }

    // Ticks the capture-timeout countdown down to zero, then fails the capture.
    Timer {
        id: countdownTimer
        interval: card.countdownTickMs
        repeat: true
        onTriggered: {
            if (card.countdown > 1) {
                card.countdown--
                card.pulseDuration = card.isCountdownCritical ? card.pulseCriticalMs : card.pulseNormalMs
            } else {
                resetRecordingState()
                card.captureFailed()
            }
        }
    }

    // ------------------------------------------------------------------
    // Animations
    // ------------------------------------------------------------------

    // Slow border pulse while the grace-period countdown is running.
    SequentialAnimation on border.width {
        running: card.isGracePeriod && card.isRecording
        loops: Animation.Infinite
        NumberAnimation { from: 1; to: 3; duration: card.pulseDuration; easing.type: Easing.InOutQuad }
        NumberAnimation { from: 3; to: 1; duration: card.pulseDuration; easing.type: Easing.InOutQuad }
    }

    // One-shot sharp flash when an unsupported key is pressed.
    SequentialAnimation {
        id: sharpErrorPulse
        NumberAnimation { target: card; property: "border.width"; from: 1; to: 3; duration: card.errorFlashInMs; easing.type: Easing.OutQuad }
        NumberAnimation { target: card; property: "border.width"; from: 3; to: 1; duration: card.errorFlashOutMs; easing.type: Easing.InQuad }
    }

    // ------------------------------------------------------------------
    // Input
    // ------------------------------------------------------------------
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onEntered: card.isHovered = true
        onExited: card.isHovered = false
        onClicked: { card.forceActiveFocus(); card.clicked() }
    }

    Keys.onPressed: (event) => {
        if (!card.isRecording) return

        if (event.key === Qt.Key_Escape) {
            card.handleCancelAction()
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Backspace) {
            card.handleClearAction()
            event.accepted = true
            return
        }
        if (card.isModifier(event.key)) return

        var seq = card.getSequence(event)
        if (seq === "") {
            card.showErrorWithMessage("UNSUPPORTED")
        } else {
            resetRecordingState()
            card.keyCaptured(seq)
        }
        event.accepted = true
    }

    // ------------------------------------------------------------------
    // Overlays
    // ------------------------------------------------------------------

    // Countdown badge, shown only during the grace-period timeout window.
    Text {
        anchors { top: parent.top; right: parent.right; margins: 12 }
        text: card.countdown
        font.pixelSize: 11
        font.weight: Font.Bold
        color: card.isCountdownCritical ? Theme.errorDefault : card.accentColor
        visible: card.isGracePeriod && card.isRecording
    }

    // Unsaved-changes indicator, hidden while actively recording.
    Image {
        anchors { top: parent.top; right: parent.right; margins: 12 }
        source: "image://icons/save?color=" + encodeURIComponent(Theme.warning)
        smooth: true
        sourceSize: Qt.size(14, 14)
        visible: card.hasUnsavedChanges && !card.isRecording
    }

    // ------------------------------------------------------------------
    // Content
    // ------------------------------------------------------------------
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 14

        // Card title (e.g. "TOGGLE MUTE")
        Text {
            text: card.title
            Layout.alignment: Qt.AlignHCenter
            font.pixelSize: 10
            font.letterSpacing: 2.5
            color: card.priorityColor(card.hasUnsavedChanges ? Theme.warning : Theme.textDim)
            Behavior on color { ColorAnimation { duration: card.colorAnimMs } }
        }

        // Main value: the bound key, an error message, or a recording indicator.
        Text {
            text: card.showError ? card.errorMessage : (card.isRecording ? "\u00B7 \u00B7 \u00B7" : (card.currentKey || "\u2014"))
            color: card.priorityColor(Theme.textPrimary)
            font.pixelSize: (card.currentKey.length > 5 || card.showError) ? 22 : 30
            font.weight: Font.Bold
            Layout.alignment: Qt.AlignHCenter
            Behavior on color { ColorAnimation { duration: card.colorAnimMs } }

            SequentialAnimation on opacity {
                running: card.isRecording && !card.showError && !card.isGracePeriod
                loops: Animation.Infinite
                NumberAnimation { from: 1.0; to: 0.2; duration: card.textFadeMs; easing.type: Easing.InOutQuad }
                NumberAnimation { from: 0.2; to: 1.0; duration: card.textFadeMs; easing.type: Easing.InOutQuad }
            }
        }

        // Small hint line: help text while recording, status line otherwise.
        Text {
            id: promptText
            text: card.showError ? "TRY AGAIN" :
                  (card.isGracePeriod ? "TIME REMAINING: 0:0" + card.countdown :
                  (card.isRecording ? card.helpText() :
                   (card.hasUnsavedChanges ? "UNSAVED CHANGES" : "CLICK TO EDIT")))
            Layout.alignment: Qt.AlignHCenter
            font.pixelSize: 9
            font.letterSpacing: 1.5
            font.weight: (card.isGracePeriod && card.isCountdownCritical) ? Font.Bold : Font.Normal

            // isGracePeriod implies isRecording, so priorityColor's "recording"
            // branch (accentColor) is what shows once the countdown is no
            // longer critical — same rule as every other element on this card.
            color: card.priorityColor(card.hasUnsavedChanges ? Theme.warning : Theme.textDim)

            opacity: card.showError ? 1.0 : (card.isHovered && !card.isRecording ? 0.8 : (card.isRecording ? 1.0 : 0.4))

            // Cross-fade the text on change while actively recording (skipped
            // during the grace period, whose countdown text updates in place).
            Behavior on text {
                enabled: card.isRecording && !card.isGracePeriod
                SequentialAnimation {
                    NumberAnimation { target: promptText; property: "opacity"; to: 0; duration: 100 }
                    PropertyAction { target: promptText; property: "text" }
                    NumberAnimation { target: promptText; property: "opacity"; to: 1; duration: 100 }
                }
            }
            Behavior on color { ColorAnimation { duration: card.colorAnimMs } }
        }
    }
}
