import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Templates 2.15 as T
import Saiko 1.0

Window {
    id: root
    width: 520
    height: 520
    minimumWidth: 520
    minimumHeight: 650
    maximumWidth: 520
    maximumHeight: 700
    color: "#0f0f0f"
    title: "Audio Routing & Settings"
    modality: Qt.ApplicationModal
    flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint

    property bool initialized: false

    signal accepted()
    signal rejected()

    function buildDeviceList(devices, showAll) {
        var virtualDevices = []
        var allDevices = []
        for (var i = 0; i < devices.length; i++) {
            var d = devices[i]
            if (d.isVirtual) virtualDevices.push(d)
            allDevices.push(d)
        }
        var list = (showAll || virtualDevices.length === 0) ? allDevices : virtualDevices
        var items = [{ text: "Default", value: "" }]
        for (var j = 0; j < list.length; j++) {
            items.push({ text: list[j].description, value: list[j].description })
        }
        return items
    }

    function selectDevice(combo, savedDescription) {
        var model = combo.model
        for (var i = 0; i < model.length; i++) {
            if (model[i].value === savedDescription) {
                combo.currentIndex = i
                return
            }
        }
        combo.currentIndex = 0
    }

    // --- Custom Components ---

    component CustomComboBox : T.ComboBox {
        id: combo
        textRole: "text"
        valueRole: "value"
        Layout.fillWidth: true
        
        implicitWidth: 120
        implicitHeight: 36
        
        contentItem: Text {
            leftPadding: 12
            rightPadding: 28
            text: combo.displayText
            font.pixelSize: 12
            color: "white"
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            implicitHeight: 36
            color: combo.hovered ? "#1a1a1a" : "#121212"
            border.color: combo.visualFocus ? "#BB86FC" : (combo.hovered ? "#2a2a2a" : "#1c1c1c")
            border.width: 1
            radius: 8
            Behavior on color { ColorAnimation { duration: 150 } }
            Behavior on border.color { ColorAnimation { duration: 150 } }

            Text {
                text: "▼"
                anchors.right: parent.right
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 8
                color: combo.hovered ? "white" : "#555"
                Behavior on color { ColorAnimation { duration: 150 } }
            }
        }

        popup: T.Popup {
            y: combo.height + 4
            width: combo.width
            implicitHeight: Math.min(contentItem.implicitHeight + 8, 200)
            padding: 4

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: combo.popup.visible ? combo.delegateModel : null
                currentIndex: combo.highlightedIndex

                ScrollBar.vertical: ScrollBar {
                    width: 6
                    policy: ScrollBar.AsNeeded
                }
            }

            background: Rectangle {
                color: "#0f0f0f"
                border.color: "#222"
                border.width: 1
                radius: 8
            }
        }

        delegate: T.ItemDelegate {
            width: combo.width - 8
            height: 34
            implicitWidth: width
            implicitHeight: height
            anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
            hoverEnabled: true
            
            contentItem: Text {
                text: modelData.text || modelData
                color: highlighted || hovered ? "white" : "#aaa"
                font.pixelSize: 12
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            
            background: Rectangle {
                color: highlighted || hovered ? "#1c1c1c" : "transparent"
                radius: 6
                Behavior on color { ColorAnimation { duration: 100 } }
            }
        }
    }

    component CustomCheckBox : T.CheckBox {
        id: cb
        font.pixelSize: 12
        hoverEnabled: true
        spacing: 12
        
        implicitWidth: contentItem.implicitWidth
        implicitHeight: Math.max(indicator.implicitHeight, contentItem.implicitHeight)

        indicator: Rectangle {
            implicitWidth: 18
            implicitHeight: 18
            x: cb.leftPadding
            y: parent.height / 2 - height / 2
            radius: 4
            color: cb.checked ? "#BB86FC" : (cb.hovered ? "#1a1a1a" : "#121212")
            border.color: cb.checked ? "#BB86FC" : (cb.hovered ? "#333" : "#1c1c1c")
            border.width: 1

            Behavior on color { ColorAnimation { duration: 150 } }
            Behavior on border.color { ColorAnimation { duration: 150 } }

            Text {
                text: "✓"
                color: "black"
                font.pixelSize: 11
                font.weight: Font.Bold
                anchors.centerIn: parent
                visible: cb.checked
            }
        }

        contentItem: Text {
            text: cb.text
            font: cb.font
            color: cb.checked ? "white" : (cb.hovered ? "#bbb" : "#888")
            leftPadding: cb.indicator.width + cb.spacing
            verticalAlignment: Text.AlignVCenter
            Behavior on color { ColorAnimation { duration: 150 } }
        }
    }

    // --- Main Layout ---

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 18

        // Title Header
        ColumnLayout {
            spacing: 4
            Text {
                text: "Audio Routing & Settings"
                color: "white"
                font.pixelSize: 22
                font.weight: Font.Bold
            }
            Text {
                text: "Configure output routes and voice passthrough devices."
                color: "#4a4a4a"
                font.pixelSize: 13
            }
        }

        // --- Soundboard Outputs Group ---
        Rectangle {
            id: outputsGroup
            Layout.fillWidth: true
            Layout.preferredHeight: outputsLayout.implicitHeight + 28
            color: "#111111"
            radius: 10
            border.color: "#1c1c1c"
            border.width: 1

            ColumnLayout {
                id: outputsLayout
                anchors.fill: parent
                anchors.margins: 14
                spacing: 12

                Text {
                    text: "SOUNDBOARD OUTPUTS"
                    color: "#555"
                    font.pixelSize: 10
                    font.letterSpacing: 1.5
                    font.weight: Font.Bold
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    CustomCheckBox {
                        id: micCb
                        text: "Enable Broadcast Output (to virtual mic)"
                        checked: qmlBackend.soundboard.micOutputEnabled
                        onToggled: qmlBackend.soundboard.setMicOutputEnabled(checked)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    Text {
                        text: "Broadcast Device:"
                        color: "#888"
                        font.pixelSize: 12
                        Layout.preferredWidth: 120
                    }
                    CustomComboBox {
                        id: micCombo
                        onCurrentValueChanged: {
                            if (initialized) qmlBackend.soundboard.setMicOutputDevice(currentValue)
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    CustomCheckBox {
                        id: localCb
                        text: "Enable Local Monitoring (hear soundboard)"
                        checked: qmlBackend.soundboard.localMonitoringEnabled
                        onToggled: qmlBackend.soundboard.setLocalMonitoringEnabled(checked)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    Text {
                        text: "Monitoring Device:"
                        color: "#888"
                        font.pixelSize: 12
                        Layout.preferredWidth: 120
                    }
                    CustomComboBox {
                        id: localCombo
                        onCurrentValueChanged: {
                            if (initialized) qmlBackend.soundboard.setLocalMonitorDevice(currentValue)
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
            color: "#111111"
            radius: 10
            border.color: "#1c1c1c"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 12

                Text {
                    text: "VOICE PASSTHROUGH (MIC INPUT)"
                    color: "#555"
                    font.pixelSize: 10
                    font.letterSpacing: 1.5
                    font.weight: Font.Bold
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    CustomCheckBox {
                        id: feedMicCb
                        text: "Feed Voice to Broadcast (Mix Soundboard + Voice)"
                        checked: qmlBackend.soundboard.micPassthroughEnabled
                        onToggled: qmlBackend.soundboard.setMicPassthroughEnabled(checked)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    Text {
                        text: "Voice Input Source:"
                        color: "#888"
                        font.pixelSize: 12
                        Layout.preferredWidth: 120
                    }
                    CustomComboBox {
                        id: voiceCombo
                        onCurrentValueChanged: {
                            if (initialized) {
                                qmlBackend.soundboard.setVoiceInputDevice(currentValue)
                                audioSource.startMonitoring(currentValue)
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 6

                    Text {
                        text: "Live Voice Input Level / Waveform:"
                        color: "#888"
                        font.pixelSize: 11
                    }

                    // Pure QML Waveform Visualizer
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: 80
                        implicitHeight: 80

                        RealtimeWaveform {
                            id: audioSource
                            width: 1
                            height: 1
                            visible: false
                        }

                        Canvas {
                            id: waveformCanvas
                            anchors.fill: parent

                            Connections {
                                target: audioSource
                                function onSamplesChanged() {
                                    waveformCanvas.requestPaint()
                                }
                            }

                            onPaint: {
                                var ctx = getContext("2d");
                                ctx.clearRect(0, 0, width, height);

                                // Background
                                var bgGrad = ctx.createLinearGradient(0, 0, 0, height);
                                bgGrad.addColorStop(0, "#121212");
                                bgGrad.addColorStop(1, "#161616");
                                ctx.fillStyle = bgGrad;
                                ctx.fillRect(0, 0, width, height);

                                // Grid lines
                                ctx.lineWidth = 1;
                                ctx.strokeStyle = "rgba(255, 255, 255, 0.03)";
                                var midY = height / 2;
                                ctx.beginPath();
                                ctx.moveTo(0, midY);
                                ctx.lineTo(width, midY);
                                ctx.moveTo(0, height * 0.25);
                                ctx.lineTo(width, height * 0.25);
                                ctx.moveTo(0, height * 0.75);
                                ctx.lineTo(width, height * 0.75);
                                ctx.stroke();

                                // Audio path line
                                var samples = audioSource.samples;
                                if (!samples || samples.length === 0) return;

                                ctx.beginPath();
                                var w = width;
                                var h = height;
                                var len = samples.length;

                                for (var i = 0; i < len; ++i) {
                                    var x = (i / (len - 1)) * w;
                                    var y = midY - (samples[i] * midY * 2.0);
                                    if (y < 0) y = 0;
                                    if (y > h) y = h;
                                    
                                    if (i === 0) {
                                        ctx.moveTo(x, y);
                                    } else {
                                        ctx.lineTo(x, y);
                                    }
                                }

                                // Stroke styling
                                var lineGrad = ctx.createLinearGradient(0, 0, w, 0);
                                lineGrad.addColorStop(0, "#03DAC6"); // Cyan glow start
                                lineGrad.addColorStop(1, "#BB86FC"); // Purple glow end
                                
                                ctx.lineWidth = 2;
                                ctx.strokeStyle = lineGrad;
                                ctx.stroke();
                            }
                        }
                    }
                }
            }
        }

        // --- Close Button ---
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight

            Rectangle {
                id: closeBtn
                Layout.alignment: Qt.AlignRight
                width: 120
                height: 38
                radius: 8
                color: closeMouse.containsMouse ? "#222" : "#1c1c1c"
                border.color: closeMouse.containsMouse ? "#333" : "#282828"
                border.width: 1
                Behavior on color { ColorAnimation { duration: 150 } }
                Behavior on border.color { ColorAnimation { duration: 150 } }

                Text {
                    anchors.centerIn: parent
                    text: "Close"
                    color: closeMouse.containsMouse ? "#fff" : "#bbb"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                MouseArea {
                    id: closeMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        audioSource.stopMonitoring()
                        root.accepted()
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        micCombo.model = buildDeviceList(qmlBackend.getAudioOutputDevices(), false)
        localCombo.model = buildDeviceList(qmlBackend.getAudioOutputDevices(), true)

        var inputs = qmlBackend.getAudioInputDevices()
        var voiceItems = [{ text: "Default Microphone", value: "" }]
        for (var i = 0; i < inputs.length; i++) {
            voiceItems.push({ text: inputs[i].description, value: inputs[i].description })
        }
        voiceCombo.model = voiceItems

        selectDevice(micCombo, qmlBackend.settings.micOutputDevice)
        selectDevice(localCombo, qmlBackend.settings.localMonitorDevice)
        selectDevice(voiceCombo, qmlBackend.settings.voiceInputDevice)

        initialized = true

        if (voiceCombo.currentValue !== undefined) {
            audioSource.startMonitoring(voiceCombo.currentValue)
        }
    }
}
