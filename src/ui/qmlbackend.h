#ifndef QMLBACKEND_H
#define QMLBACKEND_H

#include <QObject>
#include <QVariant>
#include "platform/WindowMetrics.h"
#include "models/capturestate.h"
#include "audio/waveformgenerator.h"

#include "managers/settingsmanager.h"
#include "managers/recordingmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/actionmanager.h"
#include "managers/hotkeymanager.h"
#include "models/soundplayerslotmodel.h"
#include "models/audiosourcelistmodel.h"
class QMediaPlayer;
class QAudioOutput;
class QTimer;

class QmlBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CaptureState captureState READ captureState NOTIFY captureStateChanged)
    Q_PROPERTY(QVariant replayWaveform READ replayWaveform NOTIFY replayWaveformChanged)
    Q_PROPERTY(QVariant recordingWaveform READ recordingWaveform NOTIFY recordingWaveformChanged)
    Q_PROPERTY(SettingsManager* settings READ settings CONSTANT)
    Q_PROPERTY(RecordingManager* recording READ recordingManager CONSTANT)
    Q_PROPERTY(SoundboardManager* soundboard READ soundboardManager CONSTANT)
    Q_PROPERTY(ActionManager* actions READ actionManager CONSTANT)
    Q_PROPERTY(HotkeyManager* hotkeys READ hotkeyManager CONSTANT)
    Q_PROPERTY(SoundPlayerSlotModel* slotModel READ slotModel CONSTANT)
    Q_PROPERTY(AudioSourceListModel* sourceModel READ sourceModel CONSTANT)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(qint64 playbackDuration READ playbackDuration NOTIFY playbackDurationChanged)
    Q_PROPERTY(qint64 playbackPosition READ playbackPosition NOTIFY playbackPositionChanged)
    Q_PROPERTY(int titleBarHeight READ titleBarHeight CONSTANT)
public:
    enum PlaybackType {
        PlaybackNone = 0,
        PlaybackRecording = 1,
        PlaybackReplay = 2
    };
    Q_ENUM(PlaybackType)

    explicit QmlBackend(QObject *parent = nullptr);
    ~QmlBackend();

    SettingsManager *settings() const { return m_settings; }
    RecordingManager *recordingManager() const { return m_recordingManager; }
    SoundboardManager *soundboardManager() const { return m_soundboardManager; }
    ActionManager *actionManager() const { return m_actionManager; }
    HotkeyManager *hotkeyManager() const { return m_hotkeyManager; }
    SoundPlayerSlotModel *slotModel() const { return m_slotModel; }
    AudioSourceListModel *sourceModel() const { return m_sourceModel; }

    CaptureState captureState() const;
    QVariant replayWaveform() const { return QVariant::fromValue(m_replayWaveform); }
    QVariant recordingWaveform() const { return QVariant::fromValue(m_recordingWaveform); }
    bool isPlaying() const { return m_isPlaying; }
    qint64 playbackDuration() const { return m_playbackDuration; }
    qint64 playbackPosition() const { return m_player ? m_player->position() : 0; }

    /// Canonical title bar height (single source of truth with C++ hit-testing).
    int titleBarHeight() const { return kWindowMetrics.titleBarHeight; }

    Q_INVOKABLE QVariantList getRunningProcesses() const;
    Q_INVOKABLE QStringList getProcessesProducingSound() const;
    Q_INVOKABLE QVariantList getAudioOutputDevices() const;
    Q_INVOKABLE QVariantList getAudioInputDevices() const;
    Q_INVOKABLE int systemDefaultSampleRate() const;
    Q_INVOKABLE qint64 recordingFileSize() const;
    Q_INVOKABLE void playFile(const QString &path);
    Q_INVOKABLE void stopPlayback();
    Q_INVOKABLE QString renameRecordingFile(const QString &oldPath, const QString &dir, const QString &newName);
    Q_INVOKABLE void loadRecordingWaveform(const QString &filePath);

    // Path helpers (delegates to StoragePaths for QML consumption)
    Q_INVOKABLE QString generateRecordingFilePath() const;
    Q_INVOKABLE QString generateReplayFilePath() const;
    Q_INVOKABLE QString defaultRecordingDirectory() const;
    Q_INVOKABLE QString defaultReplayDirectory() const;
    Q_INVOKABLE QString defaultBaseDirectory() const;
    Q_INVOKABLE QString logDirectory() const;

signals:
    void captureStateChanged(CaptureState state);
    void replayWaveformChanged();
    void recordingWaveformChanged();
    void playbackStateChanged();
    void playbackDurationChanged();
    void playbackPositionChanged();

private slots:
    void updateReplayWaveform();

private:
    QTimer *m_replayWaveformTimer;
    WaveformData m_replayWaveform;
    WaveformData m_recordingWaveform;
    QByteArray m_recordingPcm;
    SettingsManager *m_settings;
    RecordingManager *m_recordingManager;
    SoundboardManager *m_soundboardManager;
    ActionManager *m_actionManager;
    HotkeyManager *m_hotkeyManager;
    SoundPlayerSlotModel *m_slotModel = nullptr;
    AudioSourceListModel *m_sourceModel = nullptr;
    void *m_hotkeyBackend;
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    bool m_isPlaying = false;
    qint64 m_playbackDuration = 0;
};

#endif
