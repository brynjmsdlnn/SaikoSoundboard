#ifndef SOUNDBOARDMANAGER_H
#define SOUNDBOARDMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include "models/soundplayerslot.h"
#include "audio/soundplayer.h"
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

signals:
    void slotsChanged();
    void playerStateChanged(const QString &id, QMediaPlayer::PlaybackState state);

private:
    SettingsManager *m_settings;
    QList<SoundPlayerSlot> m_slots;
    QMap<QString, SoundPlayer*> m_players;

    void updatePlayerEngine(const SoundPlayerSlot &slot);
};

#endif // SOUNDBOARDMANAGER_H
