#ifndef RECORDINGMANAGER_H
#define RECORDINGMANAGER_H

#include <QObject>
#include <QList>
#include <QFile>
#include "audio/wasapirecorder.h"
#include "audio/audiomixer.h"
#include "audio/replaybuffer.h"
#include "audio/wavwriter.h"
#include "models/audiosource.h"
#include "managers/settingsmanager.h"
#include "models/capturestate.h"

class RecordingManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool engineRunning READ isEngineRunning NOTIFY engineRunningChanged)
    Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)
    Q_PROPERTY(CaptureState state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool isReplayActive READ isReplayActive NOTIFY replayActiveChanged)
public:
    explicit RecordingManager(SettingsManager *settings, QObject *parent = nullptr);
    ~RecordingManager();

    // Engine Control
    Q_INVOKABLE void startEngine(const QString &mode);
    Q_INVOKABLE void stopEngine();
    bool isEngineRunning() const { return !m_activeRecorders.isEmpty(); }

    // Recording Control
    Q_INVOKABLE bool startRecording(const QString &path);
    Q_INVOKABLE void stopRecording();
    bool isRecording() const { return m_wavWriter->isOpen(); }

    // Replay Control
    Q_INVOKABLE void setReplayEnabled(bool enabled, const QString &mode);
    Q_INVOKABLE bool saveReplay(const QString &path);
    Q_INVOKABLE void setReplayDuration(int seconds);

    // Accessors
    bool isReplayActive() const { return m_replayEnabled; }
    WavWriter* wavWriter() const { return m_wavWriter; }
    AudioMixer* mixer() const { return m_mixer; }
    ReplayBuffer* replayBuffer() const { return m_replayBuffer; }
    CaptureState state() const { return m_state; }

signals:
    void engineStarted();
    void engineStopped();
    void engineRunningChanged();
    void recordingStarted(const QString &path);
    void recordingStopped(const QString &path);
    void recordingChanged();
    void stateChanged(CaptureState newState);
    void replayActiveChanged();
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
