#ifndef REPLAYBUFFER_H
#define REPLAYBUFFER_H

#include <QObject>
#include "domain/RingBuffer.h"
#include "wasapirecorder.h" // For WAVEFORMATEXTENSIBLE

class ReplayBuffer : public QObject
{
    Q_OBJECT
public:
    explicit ReplayBuffer(QObject *parent = nullptr);

    void setFormat(const WAVEFORMATEXTENSIBLE& format);
    void setDuration(int seconds);
    void pushPcmChunk(const QByteArray& chunk);
    QByteArray getBufferData();
    void clear();

private:
    void updateMaxBytes();

    Saiko::Domain::RingBuffer m_ringBuffer;
    mutable QMutex m_mutex;
    int m_durationSeconds;
    WAVEFORMATEXTENSIBLE m_format;
};

#endif // REPLAYBUFFER_H
