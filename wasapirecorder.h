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

    void start(const QString &fileName, DWORD pid);
    void start(DWORD pid);
    void stop();
    bool isRunning() const { return m_running.load(); }
    
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
    std::atomic<bool> m_running;
    QFuture<void> m_future;
    qint64 m_dataChunkOffset;
    WAVEFORMATEXTENSIBLE m_format;
};

#endif // WASAPIRECORDER_H
