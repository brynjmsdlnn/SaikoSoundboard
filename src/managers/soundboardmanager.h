#ifndef SOUNDBOARDMANAGER_H
#define SOUNDBOARDMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QAudioDevice>
#include "models/soundplayerslot.h"
#include "audio/soundplayer.h"
#include "audio/waveformgenerator.h"
#include "domain/PlayerAllocator.h"
#include "managers/settingsmanager.h"

class SoundboardManager : public QObject
{
    Q_OBJECT
public:
    explicit SoundboardManager(SettingsManager *settings, QObject *parent = nullptr);
    ~SoundboardManager();

    // Player Management
    QString addPlayer(const QString &name = "New Player");
    void removePlayer(const QString &id);
    void renamePlayer(const QString &id, const QString &newName);
    void assignAudioFile(const QString &id, const QString &filePath);
    void setVolume(const QString &id, float volume);
    void setEnabled(const QString &id, bool enabled);
    void setHotkeys(const QString &id, const QString &playHotkey, const QString &assignHotkey);

    // Playback Control
    void playPlayer(const QString &id);
    void playPlayerPreview(const QString &id);
    void stopPlayer(const QString &id);
    void stopAll();

    // Replay Assignment (to be used with RecordingManager)
    void loadReplayToPlayer(const QString &id, const QString &replayPath);

    // Persistence
    void loadFromSettings();
    void saveToSettings();

    // Accessors
    QList<SoundPlayerSlot> getSlots() const { return m_slots; }
    SoundPlayerSlot* getSlot(const QString &id);
    SoundPlayer* getPlayer(const QString &id);
    SettingsManager* settings() const { return m_settings; }

    bool isMicOutputEnabled() const;
    bool isLocalMonitoringEnabled() const;

    // Mutators for Routing
    void setMicOutputEnabled(bool enabled);
    void setLocalMonitoringEnabled(bool enabled);
    void setMicOutputDevice(const QString &description);
    void setLocalMonitorDevice(const QString &description);
    void setPlayerRouting(const QString &id, OutputRouting routing);

    // Clipping and Waveforms
    void setPlayerClipRange(const QString &id, qint64 startMs, qint64 endMs);
    void loadWaveformData(const QString &playerId, const QString &filePath);
    WaveformData getWaveformData(const QString &playerId);

signals:
    void slotsChanged();
    void playerStateChanged(const QString &id, QMediaPlayer::PlaybackState state);
    void playerPositionChanged(const QString &id, qint64 position);
    void waveformGenerated(const QString &playerId, const WaveformData &data);

private:
    SettingsManager *m_settings;
    Saiko::Domain::PlayerAllocator m_allocator;
    QList<SoundPlayerSlot> m_slots;
    QMap<QString, SoundPlayer*> m_players;
    QMap<QString, WaveformData> m_waveformCache;

    QAudioDevice m_micDevice;
    QAudioDevice m_localDevice;

    void updatePlayerEngine(const SoundPlayerSlot &slot);
    QAudioDevice findAudioDevice(const QString &description);
};

#endif // SOUNDBOARDMANAGER_H
