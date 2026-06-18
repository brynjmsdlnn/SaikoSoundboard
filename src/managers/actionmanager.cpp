#include "actionmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/recordingmanager.h"
#include "managers/settingsmanager.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>

ActionManager::ActionManager(SoundboardManager *sb, RecordingManager *rec, SettingsManager *settings, QObject *parent)
    : QObject(parent), m_sb(sb), m_rec(rec), m_settings(settings)
{
}

void ActionManager::dispatch(const Action &action)
{
    switch (action.type) {
    case ActionType::PlayPlayer:
        handlePlayPlayer(action.parameters.value("playerId").toString());
        break;
    case ActionType::StopPlayer:
        handleStopPlayer(action.parameters.value("playerId").toString());
        break;
    case ActionType::AssignReplayToPlayer:
        handleAssignReplayToPlayer(action.parameters.value("playerId").toString(), action.parameters.value("preserveExisting").toBool());
        break;
    case ActionType::SaveReplay:
        handleSaveReplay();
        break;
    case ActionType::MakePermanent:
        handleMakePermanent(action.parameters.value("playerId").toString());
        break;
    }
    emit actionDispatched(action);
}

void ActionManager::handlePlayPlayer(const QString &playerId)
{
    if (m_sb) {
        m_sb->playPlayer(playerId);
    }
}

void ActionManager::handleStopPlayer(const QString &playerId)
{
    if (m_sb) {
        m_sb->stopPlayer(playerId);
    }
}

void ActionManager::handleAssignReplayToPlayer(const QString &playerId, bool preserveExisting)
{
    if (!m_rec || !m_sb || !m_settings) return;

    SoundPlayerSlot* slot = m_sb->getSlot(playerId);
    if (!slot) return;

    if (preserveExisting && !slot->filePath.isEmpty()) {
        qDebug() << "ActionManager: Slot already has an assigned sound, preserving it.";
        return;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    QString path = QDir::tempPath() + QString("/SaikoReplay_%1.wav").arg(timestamp);

    if (m_rec->saveReplay(path)) {
        m_sb->loadReplayToPlayer(playerId, path);
    } else {
        qWarning() << "ActionManager: Failed to save replay for assignment to player" << playerId;
    }
}

void ActionManager::handleSaveReplay()
{
    if (!m_rec || !m_settings) return;

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString path = m_settings->saveDirectory() + QString("/Replay_%1.wav").arg(timestamp);
    
    if (!m_rec->saveReplay(path)) {
        qWarning() << "ActionManager: Failed to save replay";
    }
}

void ActionManager::handleMakePermanent(const QString &playerId)
{
    if (!m_sb || !m_settings) return;

    SoundPlayerSlot* slot = m_sb->getSlot(playerId);
    if (!slot || slot->filePath.isEmpty()) return;

    // Check if it's currently in the temp path
    if (!slot->filePath.startsWith(QDir::tempPath())) return;

    QFileInfo fileInfo(slot->filePath);
    if (!fileInfo.exists()) return;

    QString dirPath = m_settings->saveDirectory() + "/replays";
    QDir().mkpath(dirPath);

    QString permanentPath = dirPath + "/" + fileInfo.fileName();
    
    // Copy file to permanent directory
    if (QFile::copy(slot->filePath, permanentPath)) {
        m_sb->assignAudioFile(playerId, permanentPath);
        m_sb->saveToSettings();
        qDebug() << "ActionManager: Replay made permanent at" << permanentPath;
    } else {
        qWarning() << "ActionManager: Failed to copy temporary replay to permanent path";
    }
}
