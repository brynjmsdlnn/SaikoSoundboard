#ifndef WAVEFORMGENERATOR_H
#define WAVEFORMGENERATOR_H

#include <QObject>
#include <QList>
#include <QString>
#include <QFuture>
#include <QMetaType>

#ifdef Q_OS_WIN
#include <windows.h>
#include <mmreg.h>
#endif

struct WaveformData {
    Q_GADGET
    Q_PROPERTY(QList<float> peaks MEMBER peaks CONSTANT)
    Q_PROPERTY(qint64 durationMs MEMBER durationMs CONSTANT)
    Q_PROPERTY(bool isValid MEMBER isValid CONSTANT)
    Q_PROPERTY(int sampleRate MEMBER sampleRate CONSTANT)
    Q_PROPERTY(int channels MEMBER channels CONSTANT)
    Q_PROPERTY(int resolution MEMBER resolution CONSTANT)

public:
    QList<float> peaks; // amplitude peaks normalized from 0.0 to 1.0
    qint64 durationMs = 0;
    int sampleRate = 0;
    int channels = 0;
    int resolution = 0;
    bool isValid = false;
};

Q_DECLARE_METATYPE(WaveformData)

class WaveformGenerator : public QObject
{
    Q_OBJECT
public:
    explicit WaveformGenerator(QObject *parent = nullptr);

    static WaveformData generate(const QString &filePath, int resolution = 256);
    static WaveformData generateFromPcm(const QByteArray &pcmData, const WAVEFORMATEXTENSIBLE &format, int resolution = 256);
    static WaveformData generateDummyWaveform(qint64 durationMs, int sampleRate, int channels, int resolution = 256);
};

#endif // WAVEFORMGENERATOR_H
