#ifndef REPLAYBUFFER_H
#define REPLAYBUFFER_H

#include <QObject>
#include <QByteArray>
#include <QMutex>
#include <QMutexLocker>
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

    QByteArray m_buffer;
    mutable QMutex m_mutex;
    int m_durationSeconds;
    qint64 m_maxBytes;
    WAVEFORMATEXTENSIBLE m_format;
};

#endif // REPLAYBUFFER_H
