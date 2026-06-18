#ifndef WAVEFORMGENERATOR_H
#define WAVEFORMGENERATOR_H

#include <QObject>
#include <QList>
#include <QString>
#include <QFuture>

struct WaveformData {
    QList<float> peaks; // amplitude peaks normalized from 0.0 to 1.0
    qint64 durationMs = 0;
    int sampleRate = 0;
    int channels = 0;
    int resolution = 0;
    bool isValid = false;
};

class WaveformGenerator : public QObject
{
    Q_OBJECT
public:
    explicit WaveformGenerator(QObject *parent = nullptr);

    static WaveformData generate(const QString &filePath, int resolution = 256);
    static WaveformData generateDummyWaveform(qint64 durationMs, int sampleRate, int channels, int resolution = 256);
};

#endif // WAVEFORMGENERATOR_H
