#ifndef RECORDINGMANAGER_H
#define RECORDINGMANAGER_H

#include <QObject>
#include <QList>
#include <QFile>
#include <QMap>
#include <QSet>
#include <QAudioSink>
#include <QIODevice>
#include <QMediaDevices>
#include "audio/wasapirecorder.h"
#include "audio/audiomixer.h"
#include "audio/replaybuffer.h"
#include "audio/wavwriter.h"
#include "models/audiosource.h"
#include "managers/settingsmanager.h"
#include "models/capturestate.h"

class WasapiPassthrough;
class AudioSourceListModel;

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

    // Solo Monitor
    Q_INVOKABLE void setSourceSolo(const QString &sourceId, bool solo);
    Q_INVOKABLE void setSourceVolume(const QString &sourceId, float volume);
    Q_INVOKABLE void setSourceMuted(const QString &sourceId, bool muted);

    // Source Model (injected for capture readiness)
    void setSourceModel(AudioSourceListModel *model);

    // Capture Readiness
    Q_INVOKABLE bool isCaptureModeReady(const QString &mode) const;

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
    void replaySaved(const QString &path);
    void errorOccurred(const QString &msg);
    void soloChanged(const QString &sourceId, bool solo);
    void captureReadyChanged();

private:
    AudioSourceListModel *m_sourceModel = nullptr;
    void startProcessRecorder(DWORD pid, const QString& sourceId, float volume);
    void startDeviceRecorder(const QString& deviceName, const QString& sourceId, float volume);
    void setupRecorder(WasapiRecorder *rec, const QString& sourceId, float volume);
    void updateMuteStates();
    void updateState();
    void syncDevicePassthroughs();

    SettingsManager *m_settings;
    AudioMixer *m_mixer;
    ReplayBuffer *m_replayBuffer;
    WavWriter *m_wavWriter;
    QList<WasapiRecorder*> m_activeRecorders;
    QMap<QString, DWORD> m_sourcePids;
    QMap<QString, WasapiPassthrough*> m_devicePassthroughs;
    QSet<QString> m_soloedSources;
    bool m_replayEnabled;
    CaptureState m_state;
};

#endif // RECORDINGMANAGER_H
