#include "soundboardmanager.h"
#include <QDebug>

SoundboardManager::SoundboardManager(SettingsManager *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
}

SoundboardManager::~SoundboardManager()
{
    qDeleteAll(m_players);
}

QString SoundboardManager::addPlayer(const QString &name)
{
    SoundPlayerSlot slot;
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
        if (SoundPlayer *player = getPlayer(id)) {
            player->load(filePath);
        }
        emit slotsChanged();
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
            player->play();
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
    
    // Clean up old engines
    qDeleteAll(m_players);
    m_players.clear();

    // Create new engines
    for (const auto &slot : m_slots) {
        updatePlayerEngine(slot);
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

void SoundboardManager::updatePlayerEngine(const SoundPlayerSlot &slot)
{
    if (!m_players.contains(slot.id)) {
        SoundPlayer *player = new SoundPlayer(this);
        player->setVolume(slot.volume);
        if (!slot.filePath.isEmpty()) {
            player->load(slot.filePath);
        }
        
        connect(player, &SoundPlayer::stateChanged, this, [this, id = slot.id](QMediaPlayer::PlaybackState state) {
            emit playerStateChanged(id, state);
        });
        
        m_players.insert(slot.id, player);
    }
}
