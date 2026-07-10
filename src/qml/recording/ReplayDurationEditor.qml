import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Saiko 1.0

// ── Replay Duration Editor ──────────────────────────────────────────────
// Inline display/editing of the replay buffer duration.
// Uses a single RowLayout with shared timer icon and "Duration:" label,
// and a swap zone that cross-fades between value+pencil and a SpinBox.
Item {
    id: root

    // ── Internal State ───────────────────────────────────────────────────
    property bool _editingDuration: false
    property bool _showSuccess: false

    implicitWidth: mainRow.implicitWidth
    implicitHeight: 26

    RowLayout {
        id: mainRow
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        spacing: 0

        // 1. SHARED Timer Icon (Always visible, never moves)
        Item {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 12
            Layout.preferredHeight: 12
            Layout.rightMargin: 4
            Image {
                source: "image://icons/timer?color=%23b0b0b0"
                sourceSize: Qt.size(12, 12)
                anchors.centerIn: parent
            }
        }

        // 2. SHARED "Duration:" Label
        // Expands if we are hovering OR if we are currently editing
        Item {
            Layout.alignment: Qt.AlignVCenter
            clip: true
            implicitHeight: labelText.implicitHeight
            Layout.preferredWidth: (durationHover.containsMouse || root._editingDuration) ? labelText.implicitWidth + 4 : 0
            opacity: (durationHover.containsMouse || root._editingDuration) ? 1.0 : 0.0

            Behavior on Layout.preferredWidth {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: 200
                }
            }

            Text {
                id: labelText
                text: "Duration:"
                color: Theme.textSecondary
                font.bold: true
                font.pixelSize: 12
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // 3. THE SWAP ZONE (Cross-fades Text <-> SpinBox)
        Item {
            Layout.alignment: Qt.AlignVCenter
            // Compute display width directly from children to avoid 0 at init
            readonly property int displayWidth: durationValueText.implicitWidth + 18
            Layout.preferredWidth: root._editingDuration ? 72 : displayWidth
            implicitHeight: 26

            Behavior on Layout.preferredWidth {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }

            // --- Display Elements (Value + Pencil) ---
            RowLayout {
                id: displayGroup
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 0

                opacity: !root._editingDuration ? 1.0 : 0.0
                visible: opacity > 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }

                Text {
                    id: durationValueText
                    text: Backend.settings.replayDuration + "s"
                    color: Theme.textPrimary
                    font.bold: true
                    font.pixelSize: 12
                    Layout.alignment: Qt.AlignVCenter
                    Layout.rightMargin: 6
                }

                Item {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: 12
                    Layout.preferredHeight: 12
                    Image {
                        source: "image://icons/pencil?color=%23" + (durationHover.containsMouse ? "d99a3d" : "888888")
                        sourceSize: Qt.size(12, 12)
                        anchors.centerIn: parent
                        opacity: 0.7
                    }
                }
            }

            // --- Edit Elements (SpinBox with Success States) ---
            Item {
                id: editContainer
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                implicitWidth: 72
                implicitHeight: 26

                opacity: root._editingDuration ? 1.0 : 0.0
                visible: opacity > 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }

                SpinBox {
                    id: replayDurationSpin
                    anchors.fill: parent
                    from: 1
                    to: 999
                    value: Backend.settings.replayDuration
                    editable: true

                    background: Item {}

                    property bool isDirty: contentItem.activeFocus && parseInt(contentItem.text) !== Backend.settings.replayDuration
                    property bool overLimit: {
                        var v = parseInt(contentItem.text);
                        return !isNaN(v) && v > 120;
                    }

                    // Soft cap that the buttons (not free typing) are held to
                    readonly property int softCap: 120
                    property int repeatDirection: 0

                    textFromValue: function (value) {
                        return value + "s";
                    }
                    valueFromText: function (text) {
                        var parsed = parseInt(text.replace("s", ""), 10);
                        return isNaN(parsed) ? replayDurationSpin.value : parsed;
                    }

                    leftPadding: 16
                    rightPadding: 16

                    // --- Step without persisting: only updates local value/text ---
                    function adjustValue(direction) {
                        var typed = valueFromText(contentItem.text);
                        var newValue = typed + direction * stepSize;
                        newValue = Math.max(from, Math.min(newValue, softCap));
                        if (newValue === typed)
                            return;
                        value = newValue;
                        contentItem.text = textFromValue(newValue, locale);
                        // Keep focus/editing state so isDirty styling + Enter-to-save still work
                        contentItem.forceActiveFocus();
                    }

                    function startRepeat(direction) {
                        repeatDirection = direction;
                        adjustValue(direction); // immediate first step on press
                        repeatTimer.interval = 400; // initial delay before repeating
                        repeatTimer.restart();
                    }

                    function stopRepeat() {
                        repeatTimer.stop();
                    }

                    Timer {
                        id: repeatTimer
                        repeat: true
                        onTriggered: {
                            replayDurationSpin.adjustValue(replayDurationSpin.repeatDirection);
                            interval = 80; // speed up after the first tick
                        }
                    }

                    down.indicator: Item {
                        x: 2
                        y: 0
                        z: 1
                        width: 18
                        height: parent.height
                        opacity: 1.0
                        scale: downMouse.pressed ? 0.85 : 1.0
                        Behavior on scale {
                            NumberAnimation {
                                duration: 100
                                easing.type: Easing.OutQuad
                            }
                        }
                        Image {
                            source: "image://icons/circle-minus?color=" + (downMouse.pressed ? "%23888888" : "%23b0b0b0")
                            sourceSize: Qt.size(14, 14)
                            anchors.centerIn: parent
                        }
                        MouseArea {
                            id: downMouse
                            anchors.fill: parent
                            onPressed: replayDurationSpin.startRepeat(-1)
                            onReleased: replayDurationSpin.stopRepeat()
                            onCanceled: replayDurationSpin.stopRepeat()
                        }
                    }

                    up.indicator: Item {
                        x: parent.width - width - 2
                        y: 0
                        z: 1
                        width: 18
                        height: parent.height
                        opacity: 1.0
                        scale: upMouse.pressed ? 0.85 : 1.0
                        Behavior on scale {
                            NumberAnimation {
                                duration: 100
                                easing.type: Easing.OutQuad
                            }
                        }
                        Image {
                            source: "image://icons/circle-plus?color=" + (upMouse.pressed ? "%23888888" : "%23b0b0b0")
                            sourceSize: Qt.size(14, 14)
                            anchors.centerIn: parent
                        }
                        MouseArea {
                            id: upMouse
                            anchors.fill: parent
                            onPressed: replayDurationSpin.startRepeat(1)
                            onReleased: replayDurationSpin.stopRepeat()
                            onCanceled: replayDurationSpin.stopRepeat()
                        }
                    }

                    // --- Text Input ---
                    contentItem: TextInput {
                        text: replayDurationSpin.textFromValue(replayDurationSpin.value, replayDurationSpin.locale)
                        font.bold: true
                        font.pixelSize: 12
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter

                        // Hide text while showing the success check icon;
                        // use warning color when dirty, destructiveRed when over 120
                        color: root._showSuccess
                            ? "transparent"
                            : replayDurationSpin.overLimit ? Theme.destructiveRed
                            : replayDurationSpin.isDirty ? Theme.warning
                            : Theme.textPrimary
                        selectionColor: Theme.borderDefault
                        selectByMouse: true
                        readOnly: !replayDurationSpin.editable
                        validator: IntValidator {
                            bottom: 1
                            top: 999
                        }

                        Keys.onReturnPressed: {
                            replayDurationSpin._savedOnEnter = true;
                            replayDurationSpin.saveValue();
                        }
                        Keys.onEnterPressed: {
                            replayDurationSpin._savedOnEnter = true;
                            replayDurationSpin.saveValue();
                        }

                        onEditingFinished: {
                            // Enter was handled by Keys.onReturnPressed already.
                            // On focus-loss without Enter, just close (don't persist).
                            if (!replayDurationSpin._savedOnEnter) {
                                replayDurationSpin.value = Backend.settings.replayDuration;
                                // Sync the text too — the binding was broken by explicit assignment
                                replayDurationSpin.contentItem.text = replayDurationSpin.textFromValue(Backend.settings.replayDuration, replayDurationSpin.locale);
                                root._editingDuration = false;
                            }
                            replayDurationSpin._savedOnEnter = false;
                        }
                    }

                    // --- Interactive Icons ---
                    // Success check — overlaid on top of the value text
                    Image {
                        anchors.centerIn: parent
                        source: "image://icons/check?color=" + encodeURIComponent(Theme.accentGreen)
                        sourceSize: Qt.size(16, 16)
                        opacity: root._showSuccess ? 1.0 : 0.0
                        Behavior on opacity {
                            NumberAnimation {
                                duration: 150
                            }
                        }
                    }

                    // --- Save Logic: persists only on explicit Enter/Return ---
                    // Parse value from displayed text because the SpinBox's internal `value`
                    // may lag behind when the user types directly into the text field.
                    property bool _savedOnEnter: false

                    function saveValue(showSuccess) {
                        _savedOnEnter = true;
                        var newValue = replayDurationSpin.valueFromText(contentItem.text);

                        if (replayDurationSpin.overLimit) {
                            // Refuse to save — flash the error state instead of closing/succeeding
                            errorTimer.restart();
                            return;
                        }

                        if (newValue !== Backend.settings.replayDuration) {
                            replayDurationSpin.value = newValue;
                            Backend.recording.setReplayDuration(newValue);
                            Backend.settings.replayDuration = newValue;
                            Backend.settings.save();

                            // Show success indicator only when explicitly requested (Enter/Return)
                            if (showSuccess !== false) {
                                root._showSuccess = true;
                                successTimer.restart();
                            }
                        } else if (!root._showSuccess) {
                            // If no changes, just close the editor
                            root._editingDuration = false;
                        }
                    }
                }
            }
        }
    }

    // Hover area
    MouseArea {
        id: durationHover
        anchors.fill: parent
        enabled: !root._editingDuration
        hoverEnabled: true
        cursorShape: Qt.IBeamCursor
        onClicked: {
            root._editingDuration = true;
            Qt.callLater(function () {
                replayDurationSpin.focus = true;
            });
        }
    }

    Timer {
        id: successTimer
        interval: 1500
        onTriggered: {
            root._showSuccess = false;
            root._editingDuration = false;
        }
    }

    Timer {
        id: errorTimer
        interval: 400
        onTriggered: replayDurationSpin.contentItem.forceActiveFocus()
    }
}
