#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantMap>

class SoundboardManager;
class RecordingManager;
class SettingsManager;

enum class ActionType {
    PlayPlayer,
    StopPlayer,
    AssignReplayToPlayer,
    SaveReplay
};

struct Action {
    ActionType type;
    QVariantMap parameters;

    static Action createPlay(const QString &playerId) {
        return { ActionType::PlayPlayer, {{"playerId", playerId}} };
    }
    static Action createStop(const QString &playerId) {
        return { ActionType::StopPlayer, {{"playerId", playerId}} };
    }
    static Action createAssignReplay(const QString &playerId) {
        return { ActionType::AssignReplayToPlayer, {{"playerId", playerId}} };
    }
    static Action createSaveReplay() {
        return { ActionType::SaveReplay, {} };
    }
};

class ActionManager : public QObject
{
    Q_OBJECT
public:
    explicit ActionManager(SoundboardManager *sb, RecordingManager *rec, SettingsManager *settings, QObject *parent = nullptr);

    void dispatch(const Action &action);

signals:
    void actionDispatched(const Action &action);

private:
    SoundboardManager *m_sb;
    RecordingManager *m_rec;
    SettingsManager *m_settings;

    void handlePlayPlayer(const QString &playerId);
    void handleStopPlayer(const QString &playerId);
    void handleAssignReplayToPlayer(const QString &playerId);
    void handleSaveReplay();
};

#endif // ACTIONMANAGER_H
