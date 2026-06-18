#include "replaybuffer.h"
#include <QDebug>
#include <cstring>

ReplayBuffer::ReplayBuffer(QObject *parent)
    : QObject(parent)
    , m_durationSeconds(30)
    , m_maxBytes(0)
{
    memset(&m_format, 0, sizeof(m_format));
}

void ReplayBuffer::setFormat(const WAVEFORMATEXTENSIBLE& format)
{
    QMutexLocker locker(&m_mutex);
    m_format = format;
    updateMaxBytes();
    
    // If format changes significantly, we might want to clear or adapt the buffer.
    // For now, let's clear it to avoid corruption if sample rates/channels change.
    m_buffer.clear();
}

void ReplayBuffer::setDuration(int seconds)
{
    QMutexLocker locker(&m_mutex);
    m_durationSeconds = seconds;
    updateMaxBytes();
    
    // Trim immediately if the new duration is shorter
    if (m_buffer.size() > m_maxBytes) {
        m_buffer.remove(0, m_buffer.size() - m_maxBytes);
    }
}

void ReplayBuffer::updateMaxBytes()
{
    if (m_format.Format.nSamplesPerSec == 0) {
        m_maxBytes = 0;
        return;
    }

    // Bytes per second = SampleRate * NumChannels * (BitsPerSample / 8)
    qint64 bytesPerSec = (qint64)m_format.Format.nSamplesPerSec * 
                         m_format.Format.nChannels * 
                         (m_format.Format.wBitsPerSample / 8);
    
    m_maxBytes = bytesPerSec * m_durationSeconds;
    qDebug() << "ReplayBuffer: Max size updated to" << m_maxBytes << "bytes (" << m_durationSeconds << "s)";
}

void ReplayBuffer::pushPcmChunk(const QByteArray& chunk)
{
    QMutexLocker locker(&m_mutex);
    if (m_maxBytes == 0) return;

    m_buffer.append(chunk);

    if (m_buffer.size() > m_maxBytes) {
        int overflow = m_buffer.size() - m_maxBytes;
        m_buffer.remove(0, overflow);
    }
}

QByteArray ReplayBuffer::getBufferData()
{
    QMutexLocker locker(&m_mutex);
    return m_buffer;
}

void ReplayBuffer::clear()
{
    QMutexLocker locker(&m_mutex);
    m_buffer.clear();
}
