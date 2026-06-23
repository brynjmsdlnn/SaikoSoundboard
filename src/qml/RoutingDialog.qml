import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Saiko 1.0

Window {
    id: root
    width: 520
    height: 680
    minimumWidth: 520
    minimumHeight: 650
    maximumWidth: 520
    maximumHeight: 750
    color: Theme.appBackground
    title: "Audio Routing & Settings"
    modality: Qt.ApplicationModal
    flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint

    property bool initialized: false

    signal accepted
    signal rejected

    Component.onDestruction: {
        audioSource.stopMonitoring();
    }

    function buildDeviceList(devices, showAll) {
        var virtualDevices = [];
        var allDevices = [];
        for (var i = 0; i < devices.length; i++) {
            var d = devices[i];
            if (d.isVirtual)
                virtualDevices.push(d);
            allDevices.push(d);
        }
        var list = (showAll || virtualDevices.length === 0) ? allDevices : virtualDevices;
        var items = [
            {
                text: "Default",
                value: ""
            }
        ];
        for (var j = 0; j < list.length; j++) {
            items.push({
                text: list[j].description,
                value: list[j].description
            });
        }
        return items;
    }

    function selectDevice(combo, savedDescription) {
        var model = combo.model;
        for (var i = 0; i < model.length; i++) {
            if (model[i].value === savedDescription) {
                combo.currentIndex = i;
                return;
            }
        }
        combo.currentIndex = 0;
    }

    // --- Main Layout ---

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // Title Header
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6
            Text {
                text: "Audio Routing & Settings"
                color: Theme.textPrimary
                font.pixelSize: 22
                font.weight: Font.Bold
            }
            Text {
                text: "Configure output routes and voice passthrough devices."
                color: Theme.textDim
                font.pixelSize: Theme.fontSizeHeading || 14
            }
        }

        // --- Soundboard Outputs Group ---
        Rectangle {
            id: outputsGroup
            Layout.fillWidth: true
            implicitHeight: outputsLayout.implicitHeight + 28
            color: Theme.cardBackground
            radius: Theme.cardRadius || 8
            border.color: Theme.borderDefault
            border.width: 1

            ColumnLayout {
                id: outputsLayout
                anchors.fill: parent
                anchors.margins: 16
                spacing: 14

                Text {
                    text: "SOUNDBOARD OUTPUTS"
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSizeSmall || 11
                    font.letterSpacing: 1.5
                    font.weight: Font.Bold
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    SaikoCheckBox {
                        id: micCb
                        text: "Enable Broadcast Output (to virtual mic)"
                        checked: Backend.soundboard.micOutputEnabled
                        onToggled: Backend.soundboard.setMicOutputEnabled(checked)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    Text {
                        text: "Broadcast Device:"
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSizeNormal || 13
                        Layout.preferredWidth: 130
                        Layout.alignment: Qt.AlignVCenter
                    }
                    SaikoComboBox {
                        id: micCombo
                        Layout.fillWidth: true
                        onCurrentValueChanged: {
                            if (initialized)
                                Backend.soundboard.setMicOutputDevice(currentValue);
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    SaikoCheckBox {
                        id: localCb
                        text: "Enable Local Monitoring (hear soundboard)"
                        checked: Backend.soundboard.localMonitoringEnabled
                        onToggled: Backend.soundboard.setLocalMonitoringEnabled(checked)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    Text {
                        text: "Monitoring Device:"
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSizeNormal || 13
                        Layout.preferredWidth: 130
                        Layout.alignment: Qt.AlignVCenter
                    }
                    SaikoComboBox {
                        id: localCombo
                        Layout.fillWidth: true
                        onCurrentValueChanged: {
                            if (initialized)
                                Backend.soundboard.setLocalMonitorDevice(currentValue);
                        }
                    }
                }
            }
        }

        // --- Voice Passthrough Group ---
        Rectangle {
            id: inputGroup
            Layout.fillWidth: true
            Layout.fillHeight: true

            // CRITICAL FIX: Forces the card background to always expand
            // enough to wrap its children, preventing the waveform from overflowing.
            Layout.minimumHeight: inputLayout.implicitHeight + 32

            color: Theme.cardBackground
            radius: Theme.cardRadius || 8
            border.color: Theme.borderDefault
            border.width: 1

            ColumnLayout {
                id: inputLayout
                anchors.fill: parent
                anchors.margins: 16
                spacing: 14

                Text {
                    text: "VOICE PASSTHROUGH (MIC INPUT)"
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSizeSmall || 11
                    font.letterSpacing: 1.5
                    font.weight: Font.Bold
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    SaikoCheckBox {
                        id: feedMicCb
                        text: "Feed Voice to Broadcast (Mix Soundboard + Voice)"
                        checked: Backend.soundboard.micPassthroughEnabled
                        onToggled: Backend.soundboard.setMicPassthroughEnabled(checked)
                    }
                }

                // Warning when broadcast device is the default output (speakers)
                Rectangle {
                    id: feedbackWarning
                    Layout.fillWidth: true
                    visible: feedMicCb.checked && micCombo.currentValue === ""
                    radius: Theme.borderRadius || 6
                    color: Qt.alpha(Theme.accentRed || "#FF5252", 0.08)
                    border.color: Qt.alpha(Theme.accentRed || "#FF5252", 0.25)
                    border.width: 1

                    // Derives height dynamically from text without causing a binding loop
                    implicitHeight: warningLayout.implicitHeight + 24

                    RowLayout {
                        id: warningLayout
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 10

                        Image {
                            source: "image://icons/triangle-alert?color=%23e35d5d"
                            width: 14
                            height: 14
                            Layout.alignment: Qt.AlignTop
                        }
                        Text {
                            id: warningText
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            color: Theme.textPrimary
                            font.pixelSize: Theme.fontSizeSmall || 11
                            lineHeight: 1.15
                            text: "Your microphone will play through your speakers because the Broadcast Device is set to \"Default\" (your main audio output). To avoid this, select a virtual audio cable as the Broadcast Device above."
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    Text {
                        text: "Voice Input Source:"
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSizeNormal || 13
                        Layout.preferredWidth: 130
                        Layout.alignment: Qt.AlignVCenter
                    }
                    SaikoComboBox {
                        id: voiceCombo
                        Layout.fillWidth: true
                        onCurrentValueChanged: {
                            if (initialized) {
                                Backend.soundboard.setVoiceInputDevice(currentValue);
                                audioSource.startMonitoring(currentValue);
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8

                    Text {
                        text: "Live Voice Input Level / Waveform:"
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSizeSmall || 11
                    }

                    // Pure QML Waveform Visualizer
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        // CRITICAL FIX: Give it a baseline implicitHeight so
                        // the layout can calculate bounds correctly.
                        implicitHeight: 80
                        Layout.minimumHeight: 60

                        color: Theme.appBackground
                        radius: Theme.borderRadius || 6
                        border.color: Theme.borderDefault
                        border.width: 1
                        clip: true

                        RealtimeWaveform {
                            id: audioSource
                            width: 1
                            height: 1
                            visible: false
                        }

                        Canvas {
                            id: waveformCanvas
                            anchors.fill: parent
                            anchors.margins: 2

                            Connections {
                                target: audioSource
                                function onSamplesChanged() {
                                    waveformCanvas.requestPaint();
                                }
                            }

                            onPaint: {
                                var ctx = getContext("2d");
                                ctx.clearRect(0, 0, width, height);

                                var bgGrad = ctx.createLinearGradient(0, 0, 0, height);
                                bgGrad.addColorStop(0, "#0E0E10");
                                bgGrad.addColorStop(1, "#141416");
                                ctx.fillStyle = bgGrad;
                                ctx.fillRect(0, 0, width, height);

                                ctx.lineWidth = 1;
                                ctx.strokeStyle = "rgba(255, 255, 255, 0.04)";
                                var midY = height / 2;
                                ctx.beginPath();
                                ctx.moveTo(0, midY);
                                ctx.lineTo(width, midY);
                                ctx.moveTo(0, height * 0.25);
                                ctx.lineTo(width, height * 0.25);
                                ctx.moveTo(0, height * 0.75);
                                ctx.lineTo(width, height * 0.75);
                                ctx.stroke();

                                var samples = audioSource.samples;
                                if (!samples || samples.length === 0)
                                    return;

                                ctx.beginPath();
                                var w = width;
                                var h = height;
                                var len = samples.length;

                                for (var i = 0; i < len; ++i) {
                                    var x = (i / (len - 1)) * w;
                                    var y = midY - (samples[i] * midY * 2.0);
                                    if (y < 0)
                                        y = 0;
                                    if (y > h)
                                        y = h;

                                    if (i === 0) {
                                        ctx.moveTo(x, y);
                                    } else {
                                        ctx.lineTo(x, y);
                                    }
                                }

                                var lineGrad = ctx.createLinearGradient(0, 0, w, 0);
                                lineGrad.addColorStop(0, "#00E5FF");
                                lineGrad.addColorStop(1, "#A855F7");

                                ctx.lineWidth = 2;
                                ctx.strokeStyle = lineGrad;
                                ctx.stroke();
                            }
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        micCombo.model = buildDeviceList(Backend.getAudioOutputDevices(), false);
        localCombo.model = buildDeviceList(Backend.getAudioOutputDevices(), true);

        var inputs = Backend.getAudioInputDevices();
        var voiceItems = [
            {
                text: "Default Microphone",
                value: ""
            }
        ];
        for (var i = 0; i < inputs.length; i++) {
            voiceItems.push({
                text: inputs[i].description,
                value: inputs[i].description
            });
        }
        voiceCombo.model = voiceItems;

        selectDevice(micCombo, Backend.settings.micOutputDevice);
        selectDevice(localCombo, Backend.settings.localMonitorDevice);
        selectDevice(voiceCombo, Backend.settings.voiceInputDevice);

        initialized = true;

        if (voiceCombo.currentValue !== undefined) {
            audioSource.startMonitoring(voiceCombo.currentValue);
        }
    }
}
