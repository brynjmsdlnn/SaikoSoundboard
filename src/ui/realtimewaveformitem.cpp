#include "realtimewaveformitem.h"
#include <QMediaDevices>
#include <QAudioDevice>

RealtimeWaveformItem::RealtimeWaveformItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_samples = QList<float>(m_maxSamples, 0.0f);
}

RealtimeWaveformItem::~RealtimeWaveformItem()
{
    stopMonitoring();
}

QVariantList RealtimeWaveformItem::samples() const
{
    QVariantList list;
    list.reserve(m_samples.size());
    for (float val : m_samples) {
        list.append(val);
    }
    return list;
}

void RealtimeWaveformItem::startMonitoring(const QString &deviceDescription)
{
    stopMonitoring();
    qDebug() << "startMonitoring called with:" << deviceDescription;

    QAudioDevice targetDev;
    if (deviceDescription.isEmpty()) {
        targetDev = QMediaDevices::defaultAudioInput();
    } else {
        const auto inputs = QMediaDevices::audioInputs();
        for (const auto &dev : inputs) {
            if (dev.description() == deviceDescription) {
                targetDev = dev;
                break;
            }
        }
        if (targetDev.isNull()) {
            targetDev = QMediaDevices::defaultAudioInput();
        }
    }

    if (targetDev.isNull()) {
        qDebug() << "No valid audio input device found!";
        return;
    }

    QAudioFormat format;
    format.setSampleRate(11025);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!targetDev.isFormatSupported(format)) {
        format = targetDev.preferredFormat();
    }

    m_audioSource = new QAudioSource(targetDev, format, this);
    m_audioSource->setBufferSize(1024);
    m_audioDevice = m_audioSource->start();
    qDebug() << "audioDevice valid:" << (m_audioDevice != nullptr);
    if (m_audioDevice) {
        connect(m_audioDevice, &QIODevice::readyRead, this, &RealtimeWaveformItem::onReadyRead);
    }
}

void RealtimeWaveformItem::stopMonitoring()
{
    if (m_audioSource) {
        m_audioSource->stop();
        delete m_audioSource;
        m_audioSource = nullptr;
        m_audioDevice = nullptr;
    }
    m_samples = QList<float>(m_maxSamples, 0.0f);
    emit samplesChanged();
}

void RealtimeWaveformItem::setDeviceDescription(const QString &description)
{
    if (m_deviceDescription != description) {
        m_deviceDescription = description;
        emit deviceDescriptionChanged();
        startMonitoring(description);
    }
}

void RealtimeWaveformItem::onReadyRead()
{
    if (!m_audioDevice) return;

    QByteArray data = m_audioDevice->readAll();
    if (data.isEmpty()) return;

    int sampleSize = sizeof(qint16);
    int numSamples = data.size() / sampleSize;
    if (numSamples <= 0) return;

    const qint16 *rawSamples = reinterpret_cast<const qint16*>(data.constData());

    // Downsample to roughly fill a fraction of the ring buffer per read,
    // not a fixed count of 20 regardless of buffer size.
    int targetPerRead = qMax(1, m_maxSamples / 10); // ~10 reads to refill the buffer
    int step = qMax(1, numSamples / targetPerRead);

    bool changed = false;
    for (int i = 0; i < numSamples; i += step) {
        float sampleVal = rawSamples[i] / 32768.0f;
        m_samples.removeFirst();
        m_samples.append(sampleVal);
        changed = true;
    }

    if (changed) {
        emit samplesChanged();
    }
}
