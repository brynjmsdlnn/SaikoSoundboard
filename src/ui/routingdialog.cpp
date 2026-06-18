#include "routingdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

// --- RealtimeWaveformWidget Implementation ---

RealtimeWaveformWidget::RealtimeWaveformWidget(QWidget *parent)
    : QWidget(parent)
{
    m_samples.fill(0.0f, m_maxSamples);
    setMinimumHeight(100);
}

RealtimeWaveformWidget::~RealtimeWaveformWidget()
{
    stopMonitoring();
}

void RealtimeWaveformWidget::startMonitoring(const QAudioDevice &device)
{
    stopMonitoring();

    if (device.isNull()) return;

    QAudioFormat format;
    format.setSampleRate(11025);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!device.isFormatSupported(format)) {
        format = device.preferredFormat();
    }

    m_audioSource = new QAudioSource(device, format, this);
    m_audioSource->setBufferSize(1024);
    m_audioDevice = m_audioSource->start();
    if (m_audioDevice) {
        connect(m_audioDevice, &QIODevice::readyRead, this, &RealtimeWaveformWidget::onReadyRead);
    }
}

void RealtimeWaveformWidget::stopMonitoring()
{
    if (m_audioSource) {
        m_audioSource->stop();
        delete m_audioSource;
        m_audioSource = nullptr;
        m_audioDevice = nullptr;
    }
    m_samples.fill(0.0f, m_maxSamples);
    update();
}

void RealtimeWaveformWidget::onReadyRead()
{
    if (!m_audioDevice) return;

    QByteArray data = m_audioDevice->readAll();
    if (data.isEmpty()) return;

    int sampleSize = sizeof(qint16);
    int numSamples = data.size() / sampleSize;
    if (numSamples <= 0) return;

    const qint16 *rawSamples = reinterpret_cast<const qint16*>(data.constData());
    
    int step = qMax(1, numSamples / 40);
    for (int i = 0; i < numSamples; i += step) {
        float sampleVal = rawSamples[i] / 32768.0f;
        m_samples.removeFirst();
        m_samples.append(sampleVal);
    }

    update();
}

void RealtimeWaveformWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Dark background matching main theme
    QLinearGradient bgGrad(0, 0, 0, height());
    bgGrad.setColorAt(0, QColor(25, 28, 36));
    bgGrad.setColorAt(1, QColor(32, 38, 48));
    painter.fillRect(rect(), bgGrad);

    // Subtle grid lines
    painter.setPen(QColor(60, 70, 85, 80));
    painter.drawLine(0, height() / 2, width(), height() / 2);
    painter.drawLine(0, height() / 4, width(), height() / 4);
    painter.drawLine(0, 3 * height() / 4, width(), 3 * height() / 4);

    if (m_samples.isEmpty()) return;

    QPainterPath path;
    float w = width();
    float h = height();
    float midY = h / 2.0f;

    path.moveTo(0, midY);
    for (int i = 0; i < m_samples.size(); ++i) {
        float x = (static_cast<float>(i) / (m_samples.size() - 1)) * w;
        float y = midY - (m_samples[i] * midY * 2.0f); // Boost visual sensitivity
        y = qBound(0.0f, y, h);
        path.lineTo(x, y);
    }

    QPen pen;
    pen.setWidth(2);
    QLinearGradient lineGrad(0, 0, w, 0);
    lineGrad.setColorAt(0, QColor(0, 242, 254));
    lineGrad.setColorAt(1, QColor(79, 172, 254));
    pen.setBrush(lineGrad);
    painter.setPen(pen);
    painter.drawPath(path);
}


// --- RoutingDialog Implementation ---

RoutingDialog::RoutingDialog(SoundboardManager *manager, QWidget *parent)
    : QDialog(parent)
    , m_manager(manager)
{
    setWindowTitle("Audio Routing & Settings");
    resize(500, 420);

    auto *mainLayout = new QVBoxLayout(this);

    // Group 1: Output Paths Configuration
    auto *outputsGroup = new QGroupBox("Soundboard Outputs", this);
    auto *outputsLayout = new QFormLayout(outputsGroup);

    m_micCb = new QCheckBox("Enable Broadcast Output", this);
    m_micCb->setChecked(m_manager->isMicOutputEnabled());

    m_micCombo = new QComboBox(this);
    m_micCombo->setToolTip("Select virtual audio cable output device");

    m_localCb = new QCheckBox("Enable Local Monitoring", this);
    m_localCb->setChecked(m_manager->isLocalMonitoringEnabled());

    m_localCombo = new QComboBox(this);
    m_localCombo->setToolTip("Select local speaker/headphone device");

    // Populate Output devices
    const auto outputs = QMediaDevices::audioOutputs();
    QList<QAudioDevice> virtualOutputs;
    for (const auto &device : outputs) {
        QString desc = device.description().toLower();
        if (desc.contains("cable") || 
            desc.contains("virtual") || 
            desc.contains("voicemeeter") || 
            desc.contains("vb-audio") || 
            desc.contains("sonar") || 
            desc.contains("loopback") || 
            desc.contains("vac") ||
            desc.contains("wave link") ||
            desc.contains("soundpad")) {
            virtualOutputs.append(device);
        }
    }

    m_micCombo->addItem("Default Mic Device", "");
    if (virtualOutputs.isEmpty()) {
        m_micCombo->setItemText(0, "Default Mic [No Virtual Devs]");
        for (const auto &device : outputs) {
            m_micCombo->addItem(device.description(), device.description());
        }
    } else {
        for (const auto &device : virtualOutputs) {
            m_micCombo->addItem(device.description(), device.description());
        }
    }

    m_localCombo->addItem("Default Local Device", "");
    for (const auto &device : outputs) {
        m_localCombo->addItem(device.description(), device.description());
    }

    // Select current output settings
    int micIdx = m_micCombo->findData(m_manager->settings()->micOutputDevice());
    if (micIdx >= 0) m_micCombo->setCurrentIndex(micIdx);
    int localIdx = m_localCombo->findData(m_manager->settings()->localMonitorDevice());
    if (localIdx >= 0) m_localCombo->setCurrentIndex(localIdx);

    outputsLayout->addRow(m_micCb);
    outputsLayout->addRow("Broadcast Device:", m_micCombo);
    outputsLayout->addRow(m_localCb);
    outputsLayout->addRow("Monitoring Device:", m_localCombo);

    mainLayout->addWidget(outputsGroup);

    // Group 2: Microphone Passthrough Configuration
    auto *inputGroup = new QGroupBox("Voice Passthrough (Mic Input)", this);
    auto *inputLayout = new QVBoxLayout(inputGroup);

    m_feedMicCb = new QCheckBox("Feed Voice to Broadcast (Mix Soundboard + Voice)", this);
    m_feedMicCb->setChecked(m_manager->isMicPassthroughEnabled());

    auto *voiceSelectLayout = new QHBoxLayout();
    voiceSelectLayout->addWidget(new QLabel("Voice Input Source:", this));
    m_voiceCombo = new QComboBox(this);
    
    // Populate input devices (unfiltered)
    const auto inputs = QMediaDevices::audioInputs();
    m_voiceCombo->addItem("Default Microphone", "");
    for (const auto &device : inputs) {
        m_voiceCombo->addItem(device.description(), device.description());
    }

    int voiceIdx = m_voiceCombo->findData(m_manager->settings()->voiceInputDevice());
    if (voiceIdx >= 0) m_voiceCombo->setCurrentIndex(voiceIdx);

    voiceSelectLayout->addWidget(m_voiceCombo, 1);
    inputLayout->addLayout(voiceSelectLayout);
    inputLayout->addWidget(m_feedMicCb);

    // Premium Live Input Meter
    inputLayout->addWidget(new QLabel("Live Voice Input Level / Waveform:", this));
    m_waveformWidget = new RealtimeWaveformWidget(this);
    inputLayout->addWidget(m_waveformWidget);

    mainLayout->addWidget(inputGroup);

    // Close Button at Bottom
    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    auto *closeBtn = new QPushButton("Close", this);
    closeBtn->setDefault(true);
    bottomLayout->addWidget(closeBtn);
    mainLayout->addLayout(bottomLayout);

    // Connect interactions
    connect(m_micCb, &QCheckBox::toggled, this, [this](bool checked) {
        m_manager->setMicOutputEnabled(checked);
    });
    connect(m_localCb, &QCheckBox::toggled, this, [this](bool checked) {
        m_manager->setLocalMonitoringEnabled(checked);
    });
    connect(m_feedMicCb, &QCheckBox::toggled, this, [this](bool checked) {
        m_manager->setMicPassthroughEnabled(checked);
    });
    connect(m_micCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_manager->setMicOutputDevice(m_micCombo->itemData(index).toString());
    });
    connect(m_localCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_manager->setLocalMonitorDevice(m_localCombo->itemData(index).toString());
    });
    connect(m_voiceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RoutingDialog::onVoiceDeviceChanged);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    // Initialize waveform monitoring
    onVoiceDeviceChanged(m_voiceCombo->currentIndex());
}

RoutingDialog::~RoutingDialog()
{
    m_waveformWidget->stopMonitoring();
}

void RoutingDialog::onVoiceDeviceChanged(int index)
{
    QString description = m_voiceCombo->itemData(index).toString();
    m_manager->setVoiceInputDevice(description);

    QAudioDevice targetDev;
    const auto inputs = QMediaDevices::audioInputs();
    for (const auto &dev : inputs) {
        if (dev.description() == description) {
            targetDev = dev;
            break;
        }
    }
    if (targetDev.isNull()) {
        targetDev = QMediaDevices::defaultAudioInput();
    }

    m_waveformWidget->startMonitoring(targetDev);
}
