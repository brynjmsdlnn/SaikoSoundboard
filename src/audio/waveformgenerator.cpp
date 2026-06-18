#include "audio/waveformgenerator.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <cmath>
#include <algorithm>

WaveformGenerator::WaveformGenerator(QObject *parent)
    : QObject(parent)
{
}

WaveformData WaveformGenerator::generate(const QString &filePath, int resolution)
{
    WaveformData data;
    data.resolution = resolution;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return data;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    // Read RIFF Header
    char riffId[4];
    if (file.read(riffId, 4) != 4 || memcmp(riffId, "RIFF", 4) != 0) {
        // Not a standard WAV, generate fallback
        file.close();
        return generateDummyWaveform(0, 44100, 2, resolution);
    }

    quint32 fileSize;
    in >> fileSize;

    char waveId[4];
    if (file.read(waveId, 4) != 4 || memcmp(waveId, "WAVE", 4) != 0) {
        file.close();
        return generateDummyWaveform(0, 44100, 2, resolution);
    }

    int sampleRate = 0;
    int channels = 0;
    int bitsPerSample = 0;
    int formatTag = 0;
    qint64 dataOffset = 0;
    qint64 dataSize = 0;

    // Loop through chunks to find fmt and data
    while (!file.atEnd()) {
        char chunkId[4];
        if (file.read(chunkId, 4) != 4) break;

        quint32 chunkSize;
        in >> chunkSize;

        if (memcmp(chunkId, "fmt ", 4) == 0) {
            if (chunkSize < 16) break;

            quint16 wFormatTag;
            quint16 nChannels;
            quint32 nSamplesPerSec;
            quint32 nAvgBytesPerSec;
            quint16 nBlockAlign;
            quint16 wBitsPerSample;

            in >> wFormatTag;
            in >> nChannels;
            in >> nSamplesPerSec;
            in >> nAvgBytesPerSec;
            in >> nBlockAlign;
            in >> wBitsPerSample;

            formatTag = wFormatTag;
            channels = nChannels;
            sampleRate = nSamplesPerSec;
            bitsPerSample = wBitsPerSample;

            // Skip remaining fmt chunk bytes if any
            if (chunkSize > 16) {
                file.seek(file.pos() + (chunkSize - 16));
            }
        }
        else if (memcmp(chunkId, "data", 4) == 0) {
            dataOffset = file.pos();
            dataSize = chunkSize;
            break; // Stop at data chunk
        }
        else {
            // Skip unknown chunk
            file.seek(file.pos() + chunkSize);
        }
    }

    if (dataOffset == 0 || dataSize == 0 || sampleRate == 0 || channels == 0 || bitsPerSample == 0) {
        file.close();
        return generateDummyWaveform(0, 44100, 2, resolution);
    }

    data.sampleRate = sampleRate;
    data.channels = channels;

    int bytesPerSample = bitsPerSample / 8;
    qint64 totalSamples = dataSize / (channels * bytesPerSample);
    data.durationMs = (totalSamples * 1000) / sampleRate;

    // Read samples
    file.seek(dataOffset);
    QByteArray rawData = file.read(dataSize);
    file.close();

    int numSamples = rawData.size() / bytesPerSample;
    if (numSamples == 0) {
        return generateDummyWaveform(data.durationMs, sampleRate, channels, resolution);
    }

    QList<float> allSamples;
    allSamples.reserve(numSamples / channels);

    bool isFloat = (formatTag == 3 || (formatTag == 65534 && bytesPerSample == 4));

    if (isFloat && bytesPerSample == 4) {
        const float *ptr = reinterpret_cast<const float*>(rawData.constData());
        int count = rawData.size() / sizeof(float);
        for (int i = 0; i < count; i += channels) {
            if (i + channels > count) break;
            float mix = 0;
            for (int c = 0; c < channels; ++c) {
                mix += std::abs(ptr[i + c]);
            }
            allSamples.append(mix / channels);
        }
    }
    else if (bytesPerSample == 2) { // 16-bit integer PCM
        const qint16 *ptr = reinterpret_cast<const qint16*>(rawData.constData());
        int count = rawData.size() / sizeof(qint16);
        for (int i = 0; i < count; i += channels) {
            if (i + channels > count) break;
            float mix = 0;
            for (int c = 0; c < channels; ++c) {
                mix += std::abs(static_cast<float>(ptr[i + c]) / 32768.0f);
            }
            allSamples.append(mix / channels);
        }
    }
    else if (bytesPerSample == 1) { // 8-bit unsigned PCM
        const quint8 *ptr = reinterpret_cast<const quint8*>(rawData.constData());
        int count = rawData.size();
        for (int i = 0; i < count; i += channels) {
            if (i + channels > count) break;
            float mix = 0;
            for (int c = 0; c < channels; ++c) {
                float sampleVal = (static_cast<float>(ptr[i + c]) - 128.0f) / 128.0f;
                mix += std::abs(sampleVal);
            }
            allSamples.append(mix / channels);
        }
    }
    else {
        // Fallback for unsupported formats
        return generateDummyWaveform(data.durationMs, sampleRate, channels, resolution);
    }

    int totalPcmSamples = allSamples.size();
    if (totalPcmSamples == 0) {
        return generateDummyWaveform(data.durationMs, sampleRate, channels, resolution);
    }

    double chunkSizeDouble = static_cast<double>(totalPcmSamples) / resolution;
    float maxVal = 0.0f;

    QList<float> peaks;
    peaks.reserve(resolution);

    for (int i = 0; i < resolution; ++i) {
        int startIdx = static_cast<int>(i * chunkSizeDouble);
        int endIdx = static_cast<int>((i + 1) * chunkSizeDouble);
        endIdx = std::clamp(endIdx, startIdx + 1, totalPcmSamples);

        float peak = 0.0f;
        for (int j = startIdx; j < endIdx; ++j) {
            if (allSamples[j] > peak) {
                peak = allSamples[j];
            }
        }
        peaks.append(peak);
        if (peak > maxVal) {
            maxVal = peak;
        }
    }

    // Normalize peaks
    if (maxVal > 0.00001f) {
        for (int i = 0; i < resolution; ++i) {
            peaks[i] = std::clamp(peaks[i] / maxVal, 0.0f, 1.0f);
        }
    }

    data.peaks = peaks;
    data.isValid = true;
    return data;
}

WaveformData WaveformGenerator::generateFromPcm(const QByteArray &pcmData, const WAVEFORMATEXTENSIBLE &format, int resolution)
{
    WaveformData data;
    data.resolution = resolution;

    if (format.Format.nSamplesPerSec == 0 || format.Format.nChannels == 0) {
        return generateDummyWaveform(0, 44100, 2, resolution);
    }

    data.sampleRate = format.Format.nSamplesPerSec;
    data.channels = format.Format.nChannels;

    int bytesPerSample = format.Format.wBitsPerSample / 8;
    if (bytesPerSample <= 0) bytesPerSample = 2; // Default fallback to 16-bit

    qint64 totalSamples = pcmData.size() / (data.channels * bytesPerSample);
    data.durationMs = (totalSamples * 1000) / data.sampleRate;

    int numSamples = pcmData.size() / bytesPerSample;
    if (numSamples == 0) {
        return generateDummyWaveform(data.durationMs, data.sampleRate, data.channels, resolution);
    }

    QList<float> allSamples;
    allSamples.reserve(numSamples / data.channels);

    static const GUID guidIeeeFloat = { 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
    bool isFloat = (format.Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
                   (format.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
                    memcmp(&format.SubFormat, &guidIeeeFloat, sizeof(GUID)) == 0));

    if (isFloat && bytesPerSample == 4) {
        const float *ptr = reinterpret_cast<const float*>(pcmData.constData());
        int count = pcmData.size() / sizeof(float);
        for (int i = 0; i < count; i += data.channels) {
            if (i + data.channels > count) break;
            float mix = 0;
            for (int c = 0; c < data.channels; ++c) {
                mix += std::abs(ptr[i + c]);
            }
            allSamples.append(mix / data.channels);
        }
    }
    else if (bytesPerSample == 2) { // 16-bit integer PCM
        const qint16 *ptr = reinterpret_cast<const qint16*>(pcmData.constData());
        int count = pcmData.size() / sizeof(qint16);
        for (int i = 0; i < count; i += data.channels) {
            if (i + data.channels > count) break;
            float mix = 0;
            for (int c = 0; c < data.channels; ++c) {
                mix += std::abs(static_cast<float>(ptr[i + c]) / 32768.0f);
            }
            allSamples.append(mix / data.channels);
        }
    }
    else if (bytesPerSample == 1) { // 8-bit unsigned PCM
        const quint8 *ptr = reinterpret_cast<const quint8*>(pcmData.constData());
        int count = pcmData.size();
        for (int i = 0; i < count; i += data.channels) {
            if (i + data.channels > count) break;
            float mix = 0;
            for (int c = 0; c < data.channels; ++c) {
                float sampleVal = (static_cast<float>(ptr[i + c]) - 128.0f) / 128.0f;
                mix += std::abs(sampleVal);
            }
            allSamples.append(mix / data.channels);
        }
    }
    else {
        return generateDummyWaveform(data.durationMs, data.sampleRate, data.channels, resolution);
    }

    int totalPcmSamples = allSamples.size();
    if (totalPcmSamples == 0) {
        return generateDummyWaveform(data.durationMs, data.sampleRate, data.channels, resolution);
    }

    double chunkSizeDouble = static_cast<double>(totalPcmSamples) / resolution;
    float maxVal = 0.0f;

    QList<float> peaks;
    peaks.reserve(resolution);

    for (int i = 0; i < resolution; ++i) {
        int startIdx = static_cast<int>(i * chunkSizeDouble);
        int endIdx = static_cast<int>((i + 1) * chunkSizeDouble);
        endIdx = std::clamp(endIdx, startIdx + 1, totalPcmSamples);

        float peak = 0.0f;
        for (int j = startIdx; j < endIdx; ++j) {
            if (allSamples[j] > peak) {
                peak = allSamples[j];
            }
        }
        peaks.append(peak);
        if (peak > maxVal) {
            maxVal = peak;
        }
    }

    // Normalize peaks
    if (maxVal > 0.00001f) {
        for (int i = 0; i < resolution; ++i) {
            peaks[i] = std::clamp(peaks[i] / maxVal, 0.0f, 1.0f);
        }
    }

    data.peaks = peaks;
    data.isValid = true;
    return data;
}

WaveformData WaveformGenerator::generateDummyWaveform(qint64 durationMs, int sampleRate, int channels, int resolution)
{
    WaveformData data;
    data.resolution = resolution;
    data.durationMs = durationMs > 0 ? durationMs : 5000;
    data.sampleRate = sampleRate > 0 ? sampleRate : 44100;
    data.channels = channels > 0 ? channels : 2;
    data.isValid = true;

    QList<float> peaks;
    for (int i = 0; i < resolution; ++i) {
        float t = static_cast<float>(i) / resolution;
        float envelope = std::sin(t * 3.14159f);
        float detail = 0.6f * std::abs(std::sin(t * 40.0f)) + 0.4f * std::abs(std::sin(t * 10.0f));
        peaks.append(std::clamp(envelope * detail, 0.05f, 1.0f));
    }
    data.peaks = peaks;
    return data;
}
