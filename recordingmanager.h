#ifndef RECORDINGMANAGER_H
#define RECORDINGMANAGER_H

#include <QObject>
#include <QList>
#include <QFile>
#include "wasapirecorder.h"
#include "audiomixer.h"
#include "replaybuffer.h"
#include "wavwriter.h"
#include "audiosource.h"
#include "settingsmanager.h"
#include "capturestate.h"

class RecordingManager : public QObject
{
    Q_OBJECT
public:
    explicit RecordingManager(SettingsManager *settings, QObject *parent = nullptr);
    ~RecordingManager();

    // Engine Control
    void startEngine(const QString &mode);
    void stopEngine();
    bool isEngineRunning() const { return !m_activeRecorders.isEmpty(); }

    // Recording Control
    bool startRecording(const QString &path);
    void stopRecording();
    bool isRecording() const { return m_wavWriter->isOpen(); }

    // Replay Control
    void setReplayEnabled(bool enabled, const QString &mode);
    bool saveReplay(const QString &path);
    void setReplayDuration(int seconds);

    // Accessors
    WavWriter* wavWriter() const { return m_wavWriter; }
    AudioMixer* mixer() const { return m_mixer; }
    ReplayBuffer* replayBuffer() const { return m_replayBuffer; }
    CaptureState state() const { return m_state; }

signals:
    void engineStarted();
    void engineStopped();
    void recordingStarted(const QString &path);
    void recordingStopped(const QString &path);
    void stateChanged(CaptureState newState);
    void errorOccurred(const QString &msg);

private:
    void startRecorderForPid(DWORD pid, const QString& sourceId, float volume);
    void updateState();

    SettingsManager *m_settings;
    AudioMixer *m_mixer;
    ReplayBuffer *m_replayBuffer;
    WavWriter *m_wavWriter;
    QList<WasapiRecorder*> m_activeRecorders;
    bool m_replayEnabled;
    CaptureState m_state;
};

#endif // RECORDINGMANAGER_H
