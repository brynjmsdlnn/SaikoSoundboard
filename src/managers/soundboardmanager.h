#ifndef SOUNDBOARDMANAGER_H
#define SOUNDBOARDMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QAudioDevice>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QAudioOutput>
#include "models/soundplayerslot.h"
#include "audio/soundplayer.h"
#include "audio/waveformgenerator.h"
#include "domain/PlayerAllocator.h"
#include "managers/settingsmanager.h"

class SoundboardManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<SoundPlayerSlot> slots READ getSlots NOTIFY slotsChanged)
    Q_PROPERTY(bool micOutputEnabled READ isMicOutputEnabled NOTIFY micOutputEnabledChanged)
    Q_PROPERTY(bool localMonitoringEnabled READ isLocalMonitoringEnabled NOTIFY localMonitoringEnabledChanged)
    Q_PROPERTY(bool micPassthroughEnabled READ isMicPassthroughEnabled NOTIFY micPassthroughEnabledChanged)
    Q_PROPERTY(SettingsManager* settings READ settings CONSTANT)
public:
    explicit SoundboardManager(SettingsManager *settings, QObject *parent = nullptr);
    ~SoundboardManager();

    // Player Management
    Q_INVOKABLE QString addPlayer(const QString &name = "New Player");
    Q_INVOKABLE void removePlayer(const QString &id);
    Q_INVOKABLE void renamePlayer(const QString &id, const QString &newName);
    Q_INVOKABLE void assignAudioFile(const QString &id, const QString &filePath);
    Q_INVOKABLE void promoteTempFile(const QString &id, const QString &newPath);
    Q_INVOKABLE void setVolume(const QString &id, float volume);
    Q_INVOKABLE void setHotkeys(const QString &id, const QString &playHotkey, const QString &assignHotkey);
    Q_INVOKABLE void setSlotLocked(const QString &id, bool locked);
    Q_INVOKABLE bool isSlotLocked(const QString &id) const;

    // Playback Control
    Q_INVOKABLE void playPlayer(const QString &id);
    Q_INVOKABLE void playPlayerPreview(const QString &id);
    Q_INVOKABLE void stopPlayer(const QString &id);
    Q_INVOKABLE void stopAll();

    // Replay Assignment (to be used with RecordingManager)
    Q_INVOKABLE void loadReplayToPlayer(const QString &id, const QString &replayPath);

    // Persistence
    Q_INVOKABLE void loadFromSettings();
    Q_INVOKABLE void saveToSettings();

    // Accessors
    QList<SoundPlayerSlot> getSlots() const { return m_slots; }
    SoundPlayerSlot* getSlot(const QString &id);
    SoundPlayer* getPlayer(const QString &id);
    SettingsManager* settings() const { return m_settings; }

    bool isMicOutputEnabled() const;
    bool isLocalMonitoringEnabled() const;

    // Mutators for Routing
    Q_INVOKABLE void setMicOutputEnabled(bool enabled);
    Q_INVOKABLE void setLocalMonitoringEnabled(bool enabled);
    Q_INVOKABLE void setMicOutputDevice(const QString &description);
    Q_INVOKABLE void setLocalMonitorDevice(const QString &description);
    Q_INVOKABLE void setPlayerRouting(const QString &id, OutputRouting routing);

    // Clipping and Waveforms
    Q_INVOKABLE void setPlayerClipRange(const QString &id, qint64 startMs, qint64 endMs);
    Q_INVOKABLE void loadWaveformData(const QString &playerId, const QString &filePath);
    Q_INVOKABLE WaveformData getWaveformData(const QString &playerId);

    // Microphone Passthrough
    bool isMicPassthroughEnabled() const;
    Q_INVOKABLE void setMicPassthroughEnabled(bool enabled);
    Q_INVOKABLE void setVoiceInputDevice(const QString &description);

signals:
    void slotsChanged();
    void playerStateChanged(const QString &id, QMediaPlayer::PlaybackState state);
    void playerPositionChanged(const QString &id, qint64 position);
    void playerDurationChanged(const QString &id, qint64 duration);
    void waveformGenerated(const QString &playerId, const WaveformData &data);
    void micOutputEnabledChanged();
    void localMonitoringEnabledChanged();
    void micPassthroughEnabledChanged();

private:
    SettingsManager *m_settings;
    Saiko::Domain::PlayerAllocator m_allocator;
    QList<SoundPlayerSlot> m_slots;
    QMap<QString, SoundPlayer*> m_players;
    QMap<QString, WaveformData> m_waveformCache;

    QAudioDevice m_micDevice;
    QAudioDevice m_localDevice;

    QMediaCaptureSession *m_passthroughSession = nullptr;
    QAudioInput *m_passthroughInput = nullptr;
    QAudioOutput *m_passthroughOutput = nullptr;

    void updatePlayerEngine(const SoundPlayerSlot &slot);
    QAudioDevice findAudioDevice(const QString &description);
    QAudioDevice findAudioInputDevice(const QString &description);
    void updatePassthroughEngine();
};

#endif // SOUNDBOARDMANAGER_H
