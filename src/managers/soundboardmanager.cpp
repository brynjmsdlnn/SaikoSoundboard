#include "soundboardmanager.h"
#include "logging/LogMacros.h"
#include <QUrl>
#include <QMediaDevices>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include "audio/wasapipassthrough.h"
#include "audio/soundplayer.h"
#include "managers/settingsmanager.h"

SoundboardManager::SoundboardManager(SettingsManager *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
    // Initialize device caches
    m_micDevice = findAudioDevice(m_settings->micOutputDevice());
    m_localDevice = findAudioDevice(m_settings->localMonitorDevice());

    connect(m_settings, &SettingsManager::enableMicOutputChanged, this, &SoundboardManager::micOutputEnabledChanged);
    connect(m_settings, &SettingsManager::enableLocalMonitoringChanged, this, &SoundboardManager::localMonitoringEnabledChanged);
    connect(m_settings, &SettingsManager::enableMicPassthroughChanged, this, &SoundboardManager::micPassthroughEnabledChanged);

    updatePassthroughEngine();
}

SoundboardManager::~SoundboardManager()
{
    qDeleteAll(m_players);
}

QString SoundboardManager::addPlayer(const QString &name)
{
    SoundPlayerSlot slot;
    slot.id = QString::fromStdString(m_allocator.allocateId()) + "_" + slot.id;
    slot.name = name;
    m_slots.append(slot);
    
    updatePlayerEngine(slot);
    
    LOG_INFO(LogCategory::Playback,
             QStringLiteral("[SoundboardManager] Added new player slot (id: \"%1\", defaultName: \"%2\")")
                 .arg(slot.id, name));
    
    emit slotsChanged();
    return slot.id;
}

void SoundboardManager::removePlayer(const QString &id)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (slot->locked) return;
    }
    for (int i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i].id == id) {
            m_slots.removeAt(i);
            if (m_players.contains(id)) {
                delete m_players.take(id);
            }
            LOG_INFO(LogCategory::Playback,
                     QStringLiteral("[SoundboardManager] Removed player slot (id: \"%1\")").arg(id));
            emit slotsChanged();
            return;
        }
    }
}

void SoundboardManager::renamePlayer(const QString &id, const QString &newName)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (slot->locked) return;
        LOG_INFO(LogCategory::Playback,
                 QStringLiteral("[SoundboardManager] Player slot renamed (id: \"%1\", newName: \"%2\")")
                     .arg(id, newName));
        slot->name = newName;
        emit slotsChanged();
    }
}

void SoundboardManager::assignAudioFile(const QString &id, const QString &filePath)
{
    QString localPath = filePath;
    if (filePath.startsWith("file:")) {
        localPath = QUrl(filePath).toLocalFile();
    }

    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (slot->locked) return;
        slot->filePath = localPath;
        slot->startTimeMs = 0;
        slot->endTimeMs = -1;

        LOG_INFO(LogCategory::Playback,
                 QStringLiteral("[SoundboardManager] Audio file assigned to slot (id: \"%1\", path: \"%2\")")
                     .arg(id, localPath));

        if (SoundPlayer *player = getPlayer(id)) {
            player->load(localPath);
            player->setRouting(slot->outputRouting);
            player->setGlobalOverrides(m_settings->enableMicOutput(), m_settings->enableLocalMonitoring());
            player->setDevices(m_micDevice, m_localDevice);
            player->setClipRange(slot->startTimeMs, slot->endTimeMs);
        }
        emit slotsChanged();

        // Trigger waveform loading/generation
        loadWaveformData(id, filePath);
    }
}

void SoundboardManager::promoteTempFile(const QString &id, const QString &newPath)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (slot->locked) return;
        slot->filePath = newPath;

        LOG_INFO(LogCategory::Playback,
                 QStringLiteral("[SoundboardManager] Promoted temp recording to slot (id: \"%1\", permanentPath: \"%2\")")
                     .arg(id, newPath));

        if (SoundPlayer *player = getPlayer(id)) {
            player->load(newPath);
            player->setRouting(slot->outputRouting);
            player->setGlobalOverrides(m_settings->enableMicOutput(), m_settings->enableLocalMonitoring());
            player->setDevices(m_micDevice, m_localDevice);
            player->setClipRange(slot->startTimeMs, slot->endTimeMs);
        }
        emit slotsChanged();
        saveToSettings();

        // Trigger waveform loading/generation
        loadWaveformData(id, newPath);
    }
}

void SoundboardManager::setVolume(const QString &id, float volume)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (slot->locked) return;
        LOG_DEBUG(LogCategory::Playback,
                  QStringLiteral("[SoundboardManager] Slot volume updated (id: \"%1\", volume: %2)")
                      .arg(id)
                      .arg(volume));
        slot->volume = volume;
        if (SoundPlayer *player = getPlayer(id)) {
            player->setVolume(volume);
        }
    }
}

void SoundboardManager::setSlotLocked(const QString &id, bool locked)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        LOG_INFO(LogCategory::Playback,
                 QStringLiteral("[SoundboardManager] Slot lock status changed (id: \"%1\", locked: %2)")
                     .arg(id)
                     .arg(locked));
        slot->locked = locked;
        emit slotsChanged();
        saveToSettings();
    }
}

bool SoundboardManager::isSlotLocked(const QString &id) const
{
    for (const auto &slot : m_slots) {
        if (slot.id == id) return slot.locked;
    }
    return false;
}

void SoundboardManager::setHotkeys(const QString &id, const QString &playHotkey, const QString &assignHotkey)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (slot->locked) return;
        LOG_DEBUG(LogCategory::Playback,
                  QStringLiteral("[SoundboardManager] Hotkeys configured for slot (id: \"%1\", play: \"%2\", assign: \"%3\")")
                      .arg(id, playHotkey, assignHotkey));
        slot->playHotkey = playHotkey;
        slot->assignHotkey = assignHotkey;
        emit slotsChanged();
    }
}

void SoundboardManager::playPlayer(const QString &id)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (SoundPlayer *player = getPlayer(id)) {
            player->setRouting(slot->outputRouting);
            player->setGlobalOverrides(m_settings->enableMicOutput(), m_settings->enableLocalMonitoring());
            player->setDevices(m_micDevice, m_localDevice);
            player->setClipRange(slot->startTimeMs, slot->endTimeMs);

            PlaybackMode effectiveMode = slot->playbackMode;
            if (effectiveMode == PlaybackMode::Default) {
                effectiveMode = m_settings->defaultPlaybackMode();
            }

            LOG_DEBUG(LogCategory::Playback,
                     QStringLiteral("[SoundboardManager] Playing slot (id: %1, name: \"%2\", slotMode: %3, effectiveMode: %4)")
                         .arg(id)
                         .arg(slot->name)
                         .arg(static_cast<int>(slot->playbackMode))
                         .arg(static_cast<int>(effectiveMode)));

            player->play(effectiveMode);
        }
    }
}

void SoundboardManager::playPlayerPreview(const QString &id)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (SoundPlayer *player = getPlayer(id)) {
            player->setRouting(slot->outputRouting);
            player->setGlobalOverrides(m_settings->enableMicOutput(), m_settings->enableLocalMonitoring());
            player->setDevices(m_micDevice, m_localDevice);
            player->setClipRange(slot->startTimeMs, slot->endTimeMs);
            LOG_DEBUG(LogCategory::Playback,
                      QStringLiteral("[SoundboardManager] Preview playback started (id: \"%1\", name: \"%2\")")
                          .arg(id, slot->name));
            player->playPreview();
        }
    }
}

void SoundboardManager::stopPlayer(const QString &id)
{
    if (SoundPlayer *player = getPlayer(id)) {
        LOG_DEBUG(LogCategory::Playback,
                  QStringLiteral("[SoundboardManager] Stopping player (id: \"%1\")").arg(id));
        player->stop();
    }
}

void SoundboardManager::stopAll()
{
    LOG_INFO(LogCategory::Playback,
             QStringLiteral("[SoundboardManager] Stop all playback triggered"));
    for (SoundPlayer *player : m_players) {
        player->stop();
    }
}

void SoundboardManager::loadReplayToPlayer(const QString &id, const QString &replayPath)
{
    assignAudioFile(id, replayPath);
}

void SoundboardManager::loadFromSettings()
{
    LOG_INFO(LogCategory::Playback,
             QStringLiteral("[SoundboardManager] Loading slots from settings (%1 slots)").arg(m_settings->soundBoardSlots().size()));

    m_slots = m_settings->soundBoardSlots();
    
    // Refresh device caches
    m_micDevice = findAudioDevice(m_settings->micOutputDevice());
    m_localDevice = findAudioDevice(m_settings->localMonitorDevice());

    // Clean up old engines
    int prevCount = m_players.size();
    qDeleteAll(m_players);
    m_players.clear();
    LOG_DEBUG(LogCategory::Playback,
              QStringLiteral("[SoundboardManager] Cleaned up %1 previous player engines").arg(prevCount));

    // Create new engines
    for (const auto &slot : m_slots) {
        updatePlayerEngine(slot);
        if (!slot.filePath.isEmpty()) {
            loadWaveformData(slot.id, slot.filePath);
        }
    }
    
    emit slotsChanged();
}

void SoundboardManager::saveToSettings()
{
    LOG_DEBUG(LogCategory::Playback,
              QStringLiteral("[SoundboardManager] Saving %1 slots to settings").arg(m_slots.size()));
    m_settings->setSoundBoardSlots(m_slots);
    m_settings->save();
}

SoundPlayerSlot* SoundboardManager::getSlot(const QString &id)
{
    for (auto &slot : m_slots) {
        if (slot.id == id) return &slot;
    }
    return nullptr;
}

SoundPlayer* SoundboardManager::getPlayer(const QString &id)
{
    return m_players.value(id, nullptr);
}

PlayState SoundboardManager::getPlayerPlayState(const QString &id) const
{
    SoundPlayer *player = m_players.value(id, nullptr);
    if (!player) return PlayState::Stopped;
    
    QMediaPlayer::PlaybackState state = player->playbackState();
    if (state == QMediaPlayer::PlayingState) {
        return player->isPreviewMode() ? PlayState::Preview : PlayState::Playing;
    }
    return PlayState::Stopped;
}

bool SoundboardManager::isMicOutputEnabled() const
{
    return m_settings->enableMicOutput();
}

bool SoundboardManager::isLocalMonitoringEnabled() const
{
    return m_settings->enableLocalMonitoring();
}

void SoundboardManager::setMicOutputEnabled(bool enabled)
{
    LOG_INFO(LogCategory::Playback,
             QStringLiteral("[SoundboardManager] Mic output %1").arg(enabled ? QStringLiteral("enabled") : QStringLiteral("disabled")));
    m_settings->setEnableMicOutput(enabled);
    m_settings->save();
    for (auto *player : m_players) {
        player->setGlobalOverrides(m_settings->enableMicOutput(), m_settings->enableLocalMonitoring());
    }
    emit micOutputEnabledChanged();
}

void SoundboardManager::setLocalMonitoringEnabled(bool enabled)
{
    LOG_INFO(LogCategory::Playback,
             QStringLiteral("[SoundboardManager] Local monitoring %1").arg(enabled ? QStringLiteral("enabled") : QStringLiteral("disabled")));
    m_settings->setEnableLocalMonitoring(enabled);
    m_settings->save();
    for (auto *player : m_players) {
        player->setGlobalOverrides(m_settings->enableMicOutput(), m_settings->enableLocalMonitoring());
    }
    emit localMonitoringEnabledChanged();
}

void SoundboardManager::setMicOutputDevice(const QString &description)
{
    LOG_INFO(LogCategory::Playback,
             QStringLiteral("[SoundboardManager] Mic output device set (device: \"%1\")").arg(description));
    m_settings->setMicOutputDevice(description);
    m_settings->save();
    m_micDevice = findAudioDevice(description);
    for (auto *player : m_players) {
        player->setDevices(m_micDevice, m_localDevice);
    }
    updatePassthroughEngine();
}

void SoundboardManager::setLocalMonitorDevice(const QString &description)
{
    LOG_INFO(LogCategory::Playback,
             QStringLiteral("[SoundboardManager] Local monitor device set (device: \"%1\")").arg(description));
    m_settings->setLocalMonitorDevice(description);
    m_settings->save();
    m_localDevice = findAudioDevice(description);
    for (auto *player : m_players) {
        player->setDevices(m_micDevice, m_localDevice);
    }
}

void SoundboardManager::setPlayerRouting(const QString &id, OutputRouting routing)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (slot->locked) return;
        LOG_DEBUG(LogCategory::Playback,
                  QStringLiteral("[SoundboardManager] Player routing set (id: \"%1\", routing: %2)")
                      .arg(id)
                      .arg(static_cast<int>(routing)));
        slot->outputRouting = routing;
        if (SoundPlayer *player = getPlayer(id)) {
            player->setRouting(routing);
        }
        saveToSettings();
    }
}

void SoundboardManager::setPlayerPlaybackMode(const QString &id, PlaybackMode mode)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (slot->locked) return;
        LOG_DEBUG(LogCategory::Playback,
                  QStringLiteral("[SoundboardManager] Set slot playback mode (id: %1, name: \"%2\", newMode: %3)")
                      .arg(id)
                      .arg(slot->name)
                      .arg(static_cast<int>(mode)));
        slot->playbackMode = mode;
        if (SoundPlayer *player = getPlayer(id)) {
            PlaybackMode effective = (mode == PlaybackMode::Default) ? m_settings->defaultPlaybackMode() : mode;
            player->setPlaybackMode(effective);
        }
        saveToSettings();
    }
}

void SoundboardManager::setPlayerClipRange(const QString &id, qint64 startMs, qint64 endMs, bool save)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (slot->locked) return;
        LOG_DEBUG(LogCategory::Playback,
                  QStringLiteral("[SoundboardManager] Player clip range set (id: \"%1\", startMs: %2, endMs: %3)")
                      .arg(id)
                      .arg(startMs)
                      .arg(endMs));
        slot->startTimeMs = startMs;
        slot->endTimeMs = endMs;
        if (SoundPlayer *player = getPlayer(id)) {
            player->setClipRange(startMs, endMs);
        }
        if (save) {
            saveToSettings();
        }
    }
}

void SoundboardManager::loadWaveformData(const QString &playerId, const QString &filePath)
{
    if (filePath.isEmpty()) return;

    LOG_INFO(LogCategory::Playback,
             QStringLiteral("[SoundboardManager] Waveform data generation started (id: \"%1\", file: \"%2\")")
                 .arg(playerId, filePath));

    if (m_waveformCache.contains(filePath)) {
        emit waveformGenerated(playerId, m_waveformCache[filePath]);
        return;
    }

    QFutureWatcher<WaveformData> *watcher = new QFutureWatcher<WaveformData>(this);
    connect(watcher, &QFutureWatcher<WaveformData>::finished, this, [this, watcher, playerId, filePath]() {
        WaveformData data = watcher->result();
        m_waveformCache.insert(filePath, data);
        emit waveformGenerated(playerId, data);
        watcher->deleteLater();
    });

    QFuture<WaveformData> future = QtConcurrent::run([filePath]() {
        return WaveformGenerator::generate(filePath, 256);
    });
    watcher->setFuture(future);
}

WaveformData SoundboardManager::getWaveformData(const QString &playerId)
{
    if (SoundPlayerSlot *slot = getSlot(playerId)) {
        return m_waveformCache.value(slot->filePath, WaveformData());
    }
    return WaveformData();
}

int SoundboardManager::getPlayerQueueCount(const QString &id) const
{
    if (SoundPlayer *player = m_players.value(id, nullptr)) {
        return player->remainingLoops();
    }
    return 0;
}

QVariantList SoundboardManager::getPlayerLayerPositions(const QString &id) const
{
    QVariantList positions;
    if (SoundPlayer *player = m_players.value(id, nullptr)) {
        const QList<qint64> rawPositions = player->activeLayerPositions();
        for (qint64 pos : rawPositions) {
            positions.append(pos);
        }
    }
    return positions;
}

void SoundboardManager::updatePlayerEngine(const SoundPlayerSlot &slot)
{
    if (!m_players.contains(slot.id)) {
        LOG_DEBUG(LogCategory::Playback,
                  QStringLiteral("[SoundboardManager] Creating player engine for slot (id: \"%1\", name: \"%2\", file: \"%3\")")
                      .arg(slot.id, slot.name, slot.filePath));
        SoundPlayer *player = new SoundPlayer(this);
        player->setVolume(slot.volume);
        player->setRouting(slot.outputRouting);
        PlaybackMode effective = (slot.playbackMode == PlaybackMode::Default) ? m_settings->defaultPlaybackMode() : slot.playbackMode;
        player->setPlaybackMode(effective);
        player->setGlobalOverrides(m_settings->enableMicOutput(), m_settings->enableLocalMonitoring());
        player->setDevices(m_micDevice, m_localDevice);
        player->setClipRange(slot.startTimeMs, slot.endTimeMs);

        if (!slot.filePath.isEmpty()) {
            player->load(slot.filePath);
        }
        
        connect(player, &SoundPlayer::stateChanged, this, [this, id = slot.id](QMediaPlayer::PlaybackState state) {
            emit playerStateChanged(id, state);
            emit playerPlayStateChanged(id, getPlayerPlayState(id));
        });

        connect(player, &SoundPlayer::positionChanged, this, [this, id = slot.id](qint64 pos) {
            emit playerPositionChanged(id, pos);
        });

        connect(player, &SoundPlayer::durationChanged, this, [this, id = slot.id](qint64 dur) {
            emit playerDurationChanged(id, dur);
        });

        connect(player, &SoundPlayer::remainingLoopsChanged, this, [this, id = slot.id](int count) {
            emit playerQueueCountChanged(id, count);
        });

        connect(player, &SoundPlayer::layerPositionsChanged, this, [this, id = slot.id]() {
            emit playerLayerPositionsChanged(id, getPlayerLayerPositions(id));
        });
        
        m_players.insert(slot.id, player);
    }
}

QAudioDevice SoundboardManager::findAudioDevice(const QString &description)
{
    if (description.isEmpty()) {
        return QMediaDevices::defaultAudioOutput();
    }
    const auto devices = QMediaDevices::audioOutputs();
    for (const auto &device : devices) {
        if (device.description() == description) {
            return device;
        }
    }
    return QMediaDevices::defaultAudioOutput();
}

bool SoundboardManager::isMicPassthroughEnabled() const
{
    return m_settings->enableMicPassthrough();
}

void SoundboardManager::setMicPassthroughEnabled(bool enabled)
{
    LOG_INFO(LogCategory::Playback,
             QStringLiteral("[SoundboardManager] Mic passthrough %1").arg(enabled ? QStringLiteral("enabled") : QStringLiteral("disabled")));
    m_settings->setEnableMicPassthrough(enabled);
    m_settings->save();
    updatePassthroughEngine();
    emit micPassthroughEnabledChanged();
}

void SoundboardManager::setVoiceInputDevice(const QString &description)
{
    LOG_INFO(LogCategory::Playback,
             QStringLiteral("[SoundboardManager] Voice input device set (device: \"%1\")").arg(description));
    m_settings->setVoiceInputDevice(description);
    m_settings->save();
    updatePassthroughEngine();
}

QAudioDevice SoundboardManager::findAudioInputDevice(const QString &description)
{
    if (description.isEmpty()) {
        return QMediaDevices::defaultAudioInput();
    }
    const auto devices = QMediaDevices::audioInputs();
    for (const auto &device : devices) {
        if (device.description() == description) {
            return device;
        }
    }
    return QMediaDevices::defaultAudioInput();
}

void SoundboardManager::updatePassthroughEngine()
{
    if (m_passthrough) {
        m_passthrough->stop();
        delete m_passthrough;
        m_passthrough = nullptr;
    }

    bool enabled = m_settings->enableMicPassthrough();
    if (!enabled) return;

    QString voiceDevName = m_settings->voiceInputDevice();
    QString outputDevName = m_settings->micOutputDevice();

    m_passthrough = new WasapiPassthrough(this);
    connect(m_passthrough, &WasapiPassthrough::error, this, [](const QString &msg) {
        LOG_WARN(LogCategory::Audio,
                 QStringLiteral("[Passthrough] Error occurred (message: \"%1\")").arg(msg));
    });
    m_passthrough->start(voiceDevName, outputDevName);
}
