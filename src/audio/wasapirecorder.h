#ifndef WASAPIRECORDER_H
#define WASAPIRECORDER_H

#include <QObject>
#include <QFile>
#include <QFuture>
#include <windows.h>
#include <mmreg.h>
#include <atomic>

class WasapiRecorder : public QObject
{
    Q_OBJECT
public:
    explicit WasapiRecorder(QObject *parent = nullptr);
    ~WasapiRecorder();

    void start(DWORD pid, const QString &deviceName = "");
    void stop();

    // Override the capture sample rate. Set before start().
    // sampleRate: 0 = system default, or any valid rate (22050, 44100, 48000, 96000, etc.)
    void setTargetSampleRate(int sampleRate);

    // Returns the system's current mix format sample rate (e.g. 48000).
    static int systemMixSampleRate();
    
    // Returns the audio format currently being used for capture
    WAVEFORMATEXTENSIBLE getFormat() const { return m_format; }

    bool writeWavHeader(QFile &file, const void* pwfx);
    void updateWavHeader(QFile &file);

signals:
    void error(const QString &message);
    void statsUpdated(qint64 totalBytes, double seconds);
    void finished();
    void pcmDataReady(const QByteArray &data);

private:
    void runCapture();

    QString m_fileName;
    DWORD m_processId;
    QString m_deviceName;
    std::atomic<bool> m_running;
    QFuture<void> m_future;
    qint64 m_dataChunkOffset;
    WAVEFORMATEXTENSIBLE m_format;
    int m_targetSampleRate = 0;
};

#endif // WASAPIRECORDER_H
