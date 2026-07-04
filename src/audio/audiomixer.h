#ifndef AUDIOMIXER_H
#define AUDIOMIXER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QByteArray>
#include <QTimer>
#include <QMutex>
#include <windows.h>
#include <mmreg.h>

class AudioMixer : public QObject
{
    Q_OBJECT
public:
    explicit AudioMixer(QObject *parent = nullptr);
    ~AudioMixer();

    void addSource(const QString &sourceId, float volume = 1.0f);
    void setSourceMuted(const QString &sourceId, bool muted);
    void updateVolume(const QString &sourceId, float volume);
    
    void pushPcmData(const QString &sourceId, const QByteArray &data);
    
    void start();
    void stop();
    
    void setOutputFormat(const WAVEFORMATEXTENSIBLE &format);
    WAVEFORMATEXTENSIBLE getOutputFormat() const { return m_outputFormat; }

signals:
    void mixedPcmReady(const QByteArray &data);

private slots:
    void onMixTimer();

private:
    WAVEFORMATEXTENSIBLE m_outputFormat;
    QMap<QString, QByteArray> m_sourceQueues;
    QMap<QString, float> m_sourceVolumes;
    QMap<QString, bool> m_sourceMuted;
    QTimer *m_mixTimer;
    QMutex m_mutex;
    
    bool m_running;
};

#endif // AUDIOMIXER_H
