#include "soundplayerslotmodel.h"
#include <QFile>
#include "managers/soundboardmanager.h"

SoundPlayerSlotModel::SoundPlayerSlotModel(SoundboardManager *manager, QObject *parent)
    : QAbstractListModel(parent)
    , m_manager(manager)
{
    m_slots = m_manager->getSlots();
    connect(m_manager, &SoundboardManager::slotsChanged, this, &SoundPlayerSlotModel::onSlotsChanged);
    connect(m_manager, &SoundboardManager::playerPlayStateChanged, this, [this](const QString &id, PlayState state) {
        Q_UNUSED(state);
        int row = rowForId(id);
        if (row != -1) {
            emit dataChanged(index(row, 0), index(row, 0), {PlayStateRole});
        }
    });
    connect(m_manager, &SoundboardManager::waveformGenerated, this, [this](const QString &playerId, const WaveformData &) {
        int row = rowForId(playerId);
        if (row != -1) {
            emit dataChanged(index(row, 0), index(row, 0), {DurationSecRole});
        }
    });
}

int SoundPlayerSlotModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_slots.size();
}

QVariant SoundPlayerSlotModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_slots.size())
        return {};

    const auto &slot = m_slots[index.row()];

    switch (role) {
    case SlotIdRole:        return slot.id;
    case NameRole:          return slot.name;
    case FilePathRole:      return slot.filePath;
    case PlayHotkeyRole:    return slot.playHotkey;
    case AssignHotkeyRole:  return slot.assignHotkey;
    case VolumeRole:        return slot.volume;

    case OutputRoutingRole: return static_cast<int>(slot.outputRouting);
    case PlaybackModeRole:  return static_cast<int>(slot.playbackMode);
    case StartTimeMsRole:   return slot.startTimeMs;
    case EndTimeMsRole:     return slot.endTimeMs;
    case IsTemporaryRole:
        return !slot.filePath.isEmpty() && slot.filePath.startsWith(QDir::tempPath());
    case DurationSecRole: {
        WaveformData wf = m_manager->getWaveformData(slot.id);
        return wf.isValid ? (static_cast<double>(wf.durationMs) / 1000.0) : 0.0;
    }
    case LockedRole:    return slot.locked;
    case FileExistsRole:
        if (slot.filePath.isEmpty()) return true;
        return QFile::exists(slot.filePath);
    case PlayStateRole:
        return static_cast<int>(m_manager->getPlayerPlayState(slot.id));
    default: return {};
    }
}

QHash<int, QByteArray> SoundPlayerSlotModel::roleNames() const
{
    return {
        {SlotIdRole,        "slotId"},
        {NameRole,          "slotName"},
        {FilePathRole,      "filePath"},
        {PlayHotkeyRole,    "playHotkey"},
        {AssignHotkeyRole,  "assignHotkey"},
        {VolumeRole,        "volume"},
        {OutputRoutingRole, "outputRouting"},
        {PlaybackModeRole,  "playbackMode"},
        {StartTimeMsRole,   "startTimeMs"},
        {EndTimeMsRole,     "endTimeMs"},
        {IsTemporaryRole,   "isTemporary"},
        {DurationSecRole,   "durationSec"},
        {LockedRole,        "locked"},
        {FileExistsRole,    "fileExists"},
        {PlayStateRole,     "playState"},
    };
}

void SoundPlayerSlotModel::setVolume(int row, float volume)
{
    if (row < 0 || row >= m_slots.size()) return;
    m_updating = true;
    m_slots[row].volume = volume;
    m_manager->setVolume(m_slots[row].id, volume);
    m_updating = false;
    emit dataChanged(index(row, 0), index(row, 0), {VolumeRole});
}

void SoundPlayerSlotModel::setRouting(int row, int routing)
{
    if (row < 0 || row >= m_slots.size()) return;
    m_updating = true;
    m_slots[row].outputRouting = static_cast<OutputRouting>(routing);
    m_manager->setPlayerRouting(m_slots[row].id, static_cast<OutputRouting>(routing));
    m_updating = false;
    emit dataChanged(index(row, 0), index(row, 0), {OutputRoutingRole});
}

void SoundPlayerSlotModel::setPlaybackMode(int row, int mode)
{
    if (row < 0 || row >= m_slots.size()) return;
    m_updating = true;
    PlaybackMode pm = static_cast<PlaybackMode>(mode);
    m_slots[row].playbackMode = pm;
    m_manager->setPlayerPlaybackMode(m_slots[row].id, pm);
    m_updating = false;
    emit dataChanged(index(row, 0), index(row, 0), {PlaybackModeRole});
}

void SoundPlayerSlotModel::setClipRange(int row, qint64 startMs, qint64 endMs, bool save)
{
    if (row < 0 || row >= m_slots.size()) return;
    m_updating = true;
    m_slots[row].startTimeMs = startMs;
    m_slots[row].endTimeMs = endMs;
    m_manager->setPlayerClipRange(m_slots[row].id, startMs, endMs, save);
    m_updating = false;
    emit dataChanged(index(row, 0), index(row, 0), {StartTimeMsRole, EndTimeMsRole});
}

void SoundPlayerSlotModel::onSlotsChanged()
{
    if (m_updating) return;
    QStringList oldIds;
    for (const auto &s : m_slots) oldIds << s.id;

    beginResetModel();
    m_slots = m_manager->getSlots();
    endResetModel();

    for (const auto &s : m_slots) {
        oldIds.removeOne(s.id);
    }
    for (const auto &removedId : oldIds) {
        emit slotRemoved(removedId);
    }
}

int SoundPlayerSlotModel::rowForId(const QString &id) const
{
    for (int i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i].id == id) return i;
    }
    return -1;
}

QVariantMap SoundPlayerSlotModel::get(int row) const
{
    QVariantMap result;
    if (row < 0 || row >= m_slots.size()) return result;
    QHash<int, QByteArray> roles = roleNames();
    QModelIndex idx = index(row, 0);
    for (auto it = roles.begin(); it != roles.end(); ++it) {
        result[QString::fromUtf8(it.value())] = data(idx, it.key());
    }
    return result;
}
