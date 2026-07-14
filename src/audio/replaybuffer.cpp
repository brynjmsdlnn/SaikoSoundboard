#include "audio/replaybuffer.h"
#include "logging/LogMacros.h"
#include <cstring>

ReplayBuffer::ReplayBuffer(QObject *parent)
    : QObject(parent)
    , m_durationSeconds(30)
{
    memset(&m_format, 0, sizeof(m_format));
}

void ReplayBuffer::setFormat(const WAVEFORMATEXTENSIBLE& format)
{
    QMutexLocker locker(&m_mutex);
    m_format = format;
    updateMaxBytes();
    
    m_ringBuffer.clear();
}

void ReplayBuffer::setDuration(int seconds)
{
    QMutexLocker locker(&m_mutex);
    m_durationSeconds = seconds;
    updateMaxBytes();
}

void ReplayBuffer::updateMaxBytes()
{
    if (m_format.Format.nSamplesPerSec == 0) {
        m_ringBuffer.setMaxBytes(0);
        return;
    }

    qint64 bytesPerSec = (qint64)m_format.Format.nSamplesPerSec * 
                         m_format.Format.nChannels * 
                         (m_format.Format.wBitsPerSample / 8);
    
    qint64 maxBytes = bytesPerSec * m_durationSeconds;
    m_ringBuffer.setMaxBytes(static_cast<size_t>(maxBytes));
    LOG_DEBUG(LogCategory::Replay,
             QStringLiteral("ReplayBuffer: Max size updated to %1 bytes (%2s)")
                 .arg(maxBytes).arg(m_durationSeconds));
}

void ReplayBuffer::pushPcmChunk(const QByteArray& chunk)
{
    QMutexLocker locker(&m_mutex);
    m_ringBuffer.push(reinterpret_cast<const uint8_t*>(chunk.data()), chunk.size());
}

QByteArray ReplayBuffer::getBufferData()
{
    QMutexLocker locker(&m_mutex);
    const auto& data = m_ringBuffer.data();
    return QByteArray(reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size()));
}

void ReplayBuffer::clear()
{
    QMutexLocker locker(&m_mutex);
    m_ringBuffer.clear();
}
