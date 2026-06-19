#include "soundboardmanager.h"
#include <QDebug>
#include <QMediaDevices>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

SoundboardManager::SoundboardManager(SettingsManager *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
    // Initialize device caches
    m_micDevice = findAudioDevice(m_settings->micOutputDevice());
    m_localDevice = findAudioDevice(m_settings->localMonitorDevice());

    // Create passthrough loopback session
    m_passthroughSession = new QMediaCaptureSession(this);
    m_passthroughInput = new QAudioInput(this);
    m_passthroughOutput = new QAudioOutput(this);

    m_passthroughSession->setAudioInput(m_passthroughInput);
    m_passthroughSession->setAudioOutput(m_passthroughOutput);

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
    
    emit slotsChanged();
    return slot.id;
}

void SoundboardManager::removePlayer(const QString &id)
{
    for (int i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i].id == id) {
            m_slots.removeAt(i);
            if (m_players.contains(id)) {
                delete m_players.take(id);
            }
            emit slotsChanged();
            return;
        }
    }
}

void SoundboardManager::renamePlayer(const QString &id, const QString &newName)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        slot->name = newName;
        emit slotsChanged();
    }
}

void SoundboardManager::assignAudioFile(const QString &id, const QString &filePath)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        slot->filePath = filePath;
        slot->startTimeMs = 0;
        slot->endTimeMs = -1;

        if (SoundPlayer *player = getPlayer(id)) {
            player->load(filePath);
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
        slot->filePath = newPath;

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
        slot->volume = volume;
        if (SoundPlayer *player = getPlayer(id)) {
            player->setVolume(volume);
        }
    }
}

void SoundboardManager::setEnabled(const QString &id, bool enabled)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        slot->enabled = enabled;
        emit slotsChanged();
    }
}

void SoundboardManager::setHotkeys(const QString &id, const QString &playHotkey, const QString &assignHotkey)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        slot->playHotkey = playHotkey;
        slot->assignHotkey = assignHotkey;
        emit slotsChanged();
    }
}

void SoundboardManager::playPlayer(const QString &id)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (!slot->enabled) return;
        if (SoundPlayer *player = getPlayer(id)) {
            player->setRouting(slot->outputRouting);
            player->setGlobalOverrides(m_settings->enableMicOutput(), m_settings->enableLocalMonitoring());
            player->setDevices(m_micDevice, m_localDevice);
            player->setClipRange(slot->startTimeMs, slot->endTimeMs);
            player->play();
        }
    }
}

void SoundboardManager::playPlayerPreview(const QString &id)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        if (!slot->enabled) return;
        if (SoundPlayer *player = getPlayer(id)) {
            player->setRouting(slot->outputRouting);
            player->setGlobalOverrides(m_settings->enableMicOutput(), m_settings->enableLocalMonitoring());
            player->setDevices(m_micDevice, m_localDevice);
            player->setClipRange(slot->startTimeMs, slot->endTimeMs);
            player->playPreview();
        }
    }
}

void SoundboardManager::stopPlayer(const QString &id)
{
    if (SoundPlayer *player = getPlayer(id)) {
        player->stop();
    }
}

void SoundboardManager::stopAll()
{
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
    m_slots = m_settings->soundBoardSlots();
    
    // Refresh device caches
    m_micDevice = findAudioDevice(m_settings->micOutputDevice());
    m_localDevice = findAudioDevice(m_settings->localMonitorDevice());

    // Clean up old engines
    qDeleteAll(m_players);
    m_players.clear();

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
    m_settings->setEnableMicOutput(enabled);
    m_settings->save();
    for (auto *player : m_players) {
        player->setGlobalOverrides(m_settings->enableMicOutput(), m_settings->enableLocalMonitoring());
    }
    emit micOutputEnabledChanged();
}

void SoundboardManager::setLocalMonitoringEnabled(bool enabled)
{
    m_settings->setEnableLocalMonitoring(enabled);
    m_settings->save();
    for (auto *player : m_players) {
        player->setGlobalOverrides(m_settings->enableMicOutput(), m_settings->enableLocalMonitoring());
    }
    emit localMonitoringEnabledChanged();
}

void SoundboardManager::setMicOutputDevice(const QString &description)
{
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
        slot->outputRouting = routing;
        if (SoundPlayer *player = getPlayer(id)) {
            player->setRouting(routing);
        }
        saveToSettings();
    }
}

void SoundboardManager::setPlayerClipRange(const QString &id, qint64 startMs, qint64 endMs)
{
    if (SoundPlayerSlot *slot = getSlot(id)) {
        slot->startTimeMs = startMs;
        slot->endTimeMs = endMs;
        if (SoundPlayer *player = getPlayer(id)) {
            player->setClipRange(startMs, endMs);
        }
        saveToSettings();
    }
}

void SoundboardManager::loadWaveformData(const QString &playerId, const QString &filePath)
{
    if (filePath.isEmpty()) return;

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

void SoundboardManager::updatePlayerEngine(const SoundPlayerSlot &slot)
{
    if (!m_players.contains(slot.id)) {
        SoundPlayer *player = new SoundPlayer(this);
        player->setVolume(slot.volume);
        player->setRouting(slot.outputRouting);
        player->setGlobalOverrides(m_settings->enableMicOutput(), m_settings->enableLocalMonitoring());
        player->setDevices(m_micDevice, m_localDevice);
        player->setClipRange(slot.startTimeMs, slot.endTimeMs);

        if (!slot.filePath.isEmpty()) {
            player->load(slot.filePath);
        }
        
        connect(player, &SoundPlayer::stateChanged, this, [this, id = slot.id](QMediaPlayer::PlaybackState state) {
            emit playerStateChanged(id, state);
        });

        connect(player, &SoundPlayer::positionChanged, this, [this, id = slot.id](qint64 pos) {
            emit playerPositionChanged(id, pos);
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
    m_settings->setEnableMicPassthrough(enabled);
    m_settings->save();
    updatePassthroughEngine();
    emit micPassthroughEnabledChanged();
}

void SoundboardManager::setVoiceInputDevice(const QString &description)
{
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
    bool enabled = m_settings->enableMicPassthrough();
    QString voiceDevName = m_settings->voiceInputDevice();

    if (enabled) {
        QAudioDevice inputDev = findAudioInputDevice(voiceDevName);
        QAudioDevice outputDev = m_micDevice;

        m_passthroughInput->setDevice(inputDev);
        m_passthroughOutput->setDevice(outputDev);

        m_passthroughInput->setMuted(false);
        m_passthroughOutput->setVolume(1.0);
    } else {
        m_passthroughInput->setMuted(true);
        m_passthroughOutput->setVolume(0.0);
    }
}
