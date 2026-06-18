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
    SaveReplay,
    MakePermanent
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
    static Action createAssignReplay(const QString &playerId, bool preserveExisting = false) {
        return { ActionType::AssignReplayToPlayer, {{"playerId", playerId}, {"preserveExisting", preserveExisting}} };
    }
    static Action createSaveReplay() {
        return { ActionType::SaveReplay, {} };
    }
    static Action createMakePermanent(const QString &playerId, const QString &customFileName = "") {
        return { ActionType::MakePermanent, {{"playerId", playerId}, {"customFileName", customFileName}} };
    }
};

Q_DECLARE_METATYPE(Action)

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
    void handleAssignReplayToPlayer(const QString &playerId, bool preserveExisting);
    void handleSaveReplay();
    void handleMakePermanent(const QString &playerId, const QString &customFileName);
};

#endif // ACTIONMANAGER_H
