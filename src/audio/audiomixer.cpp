#include "audio/audiomixer.h"
#include <QDebug>
#include <algorithm>

AudioMixer::AudioMixer(QObject *parent) : QObject(parent), m_running(false)
{
    memset(&m_outputFormat, 0, sizeof(m_outputFormat));
    m_mixTimer = new QTimer(this);
    m_mixTimer->setInterval(20); // 20ms mixing interval
    connect(m_mixTimer, &QTimer::timeout, this, &AudioMixer::onMixTimer);
}

AudioMixer::~AudioMixer()
{
    stop();
}

void AudioMixer::addSource(const QString &sourceId, float volume)
{
    QMutexLocker lock(&m_mutex);
    m_sourceQueues[sourceId] = QByteArray();
    m_sourceVolumes[sourceId] = volume;
}

void AudioMixer::removeSource(const QString &sourceId)
{
    QMutexLocker lock(&m_mutex);
    m_sourceQueues.remove(sourceId);
    m_sourceVolumes.remove(sourceId);
}

void AudioMixer::updateVolume(const QString &sourceId, float volume)
{
    QMutexLocker lock(&m_mutex);
    if (m_sourceVolumes.contains(sourceId)) {
        m_sourceVolumes[sourceId] = volume;
    }
}

void AudioMixer::pushPcmData(const QString &sourceId, const QByteArray &data)
{
    QMutexLocker lock(&m_mutex);
    if (m_sourceQueues.contains(sourceId)) {
        m_sourceQueues[sourceId].append(data);
    }
}

void AudioMixer::start()
{
    m_running = true;
    m_mixTimer->start();
}

void AudioMixer::stop()
{
    m_running = false;
    m_mixTimer->stop();
    
    QMutexLocker lock(&m_mutex);
    for (auto it = m_sourceQueues.begin(); it != m_sourceQueues.end(); ++it) {
        it.value().clear();
    }
}

void AudioMixer::setOutputFormat(const WAVEFORMATEXTENSIBLE &format)
{
    QMutexLocker lock(&m_mutex);
    m_outputFormat = format;
}

void AudioMixer::onMixTimer()
{
    QMutexLocker lock(&m_mutex);
    if (!m_running || m_outputFormat.Format.nChannels == 0) return;

    // Calculate how many bytes we need for 20ms
    // samples_per_sec * channels * bytes_per_sample * 0.02
    int bytesNeeded = m_outputFormat.Format.nSamplesPerSec * m_outputFormat.Format.nBlockAlign * 0.02;
    
    // Ensure it's aligned to sample size
    bytesNeeded -= (bytesNeeded % m_outputFormat.Format.nBlockAlign);

    if (bytesNeeded <= 0) return;

    QByteArray mixedData;
    mixedData.resize(bytesNeeded);
    
    // We assume 32-bit float or 16-bit PCM based on current WasapiRecorder behavior (EXTENSIBLE usually means float if autoconvert is used)
    // However, our code uses nBlockAlign which is usually 4 (mono 16 -> float?) or 8 (stereo float).
    // Let's assume IEEE Float (32-bit) since we use AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
    
    bool isFloat = (m_outputFormat.Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT || 
                   (m_outputFormat.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE && 
                    m_outputFormat.Format.nBlockAlign / m_outputFormat.Format.nChannels == 4));

    if (isFloat) {
        float *outBuf = reinterpret_cast<float*>(mixedData.data());
        int numSamples = bytesNeeded / sizeof(float);
        std::fill(outBuf, outBuf + numSamples, 0.0f);

        for (auto it = m_sourceQueues.begin(); it != m_sourceQueues.end(); ++it) {
            const QString &id = it.key();
            QByteArray &queue = it.value();
            float vol = m_sourceVolumes[id];

            int bytesToPull = std::min((int)queue.size(), bytesNeeded);
            if (bytesToPull > 0) {
                const float *srcBuf = reinterpret_cast<const float*>(queue.constData());
                int samplesToPull = bytesToPull / sizeof(float);
                for (int i = 0; i < samplesToPull; ++i) {
                    outBuf[i] += srcBuf[i] * vol;
                }
                queue.remove(0, bytesToPull);
            }
        }
        
        // Clamping (though float can exceed 1.0, it's safer for conversion later)
        for (int i = 0; i < numSamples; ++i) {
            if (outBuf[i] > 1.0f) outBuf[i] = 1.0f;
            else if (outBuf[i] < -1.0f) outBuf[i] = -1.0f;
        }
    } else {
        // Fallback to 16-bit integer mixing
        short *outBuf = reinterpret_cast<short*>(mixedData.data());
        int numSamples = bytesNeeded / sizeof(short);
        std::fill(outBuf, outBuf + numSamples, 0);

        for (auto it = m_sourceQueues.begin(); it != m_sourceQueues.end(); ++it) {
            const QString &id = it.key();
            QByteArray &queue = it.value();
            float vol = m_sourceVolumes[id];

            int bytesToPull = std::min((int)queue.size(), bytesNeeded);
            if (bytesToPull > 0) {
                const short *srcBuf = reinterpret_cast<const short*>(queue.constData());
                int samplesToPull = bytesToPull / sizeof(short);
                for (int i = 0; i < samplesToPull; ++i) {
                    int mixed = outBuf[i] + (int)(srcBuf[i] * vol);
                    outBuf[i] = (short)std::clamp(mixed, -32768, 32767);
                }
                queue.remove(0, bytesToPull);
            }
        }
    }

    emit mixedPcmReady(mixedData);
}
