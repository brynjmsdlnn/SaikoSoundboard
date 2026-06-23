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
        handleAssignReplayToPlayer(action.parameters.value("playerId").toString());
        break;
    case ActionType::SaveReplay:
        handleSaveReplay();
        break;
    case ActionType::MakePermanent:
        handleMakePermanent(action.parameters.value("playerId").toString(), action.parameters.value("customFileName").toString());
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

void ActionManager::handleAssignReplayToPlayer(const QString &playerId)
{
    if (!m_rec || !m_sb || !m_settings) return;

    SoundPlayerSlot* slot = m_sb->getSlot(playerId);
    if (!slot) return;

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
    QString path = m_settings->replayDirectory() + QString("/Replay_%1.wav").arg(timestamp);
    
    if (!m_rec->saveReplay(path)) {
        qWarning() << "ActionManager: Failed to save replay";
    }
}

// --- Q_INVOKABLE convenience methods ---

void ActionManager::dispatchPlay(const QString &playerId)
{
    dispatch(Action::createPlay(playerId));
}

void ActionManager::dispatchStop(const QString &playerId)
{
    dispatch(Action::createStop(playerId));
}

void ActionManager::dispatchAssignReplay(const QString &playerId)
{
    dispatch(Action::createAssignReplay(playerId));
}

void ActionManager::dispatchSaveReplay()
{
    dispatch(Action::createSaveReplay());
}

void ActionManager::dispatchMakePermanent(const QString &playerId, const QString &customFileName)
{
    dispatch(Action::createMakePermanent(playerId, customFileName));
}

void ActionManager::handleMakePermanent(const QString &playerId, const QString &customFileName)
{
    if (!m_sb || !m_settings) return;

    SoundPlayerSlot* slot = m_sb->getSlot(playerId);
    if (!slot || slot->filePath.isEmpty()) return;

    // Check if it's currently in the temp path
    if (!slot->filePath.startsWith(QDir::tempPath())) return;

    QFileInfo fileInfo(slot->filePath);
    if (!fileInfo.exists()) return;

    QString dirPath = m_settings->replayDirectory();
    QDir().mkpath(dirPath);

    QString targetName = customFileName.isEmpty() ? fileInfo.fileName() : customFileName;
    if (!targetName.contains('.'))
        targetName += "." + fileInfo.suffix();
    QString permanentPath = dirPath + "/" + targetName;
    
    if (QFile::exists(permanentPath)) {
        QFile::remove(permanentPath);
    }

    // Copy file to permanent directory
    if (QFile::copy(slot->filePath, permanentPath)) {
        m_sb->promoteTempFile(playerId, permanentPath);
        qDebug() << "ActionManager: Replay made permanent at" << permanentPath;
    } else {
        qWarning() << "ActionManager: Failed to copy temporary replay to permanent path";
    }
}
