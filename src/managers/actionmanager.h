#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantMap>

class SoundboardManager;
class RecordingManager;
class SettingsManager;

namespace SaikoActions {

Q_NAMESPACE

enum class ActionType {
    PlayPlayer,
    StopPlayer,
    AssignReplayToPlayer,
    SaveReplay,
    MakePermanent
};
Q_ENUM_NS(ActionType)

} // namespace SaikoActions

using SaikoActions::ActionType;

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

    Q_INVOKABLE void dispatch(const Action &action);

    // Q_INVOKABLE convenience methods — QML can't easily construct Action structs
    Q_INVOKABLE void dispatchPlay(const QString &playerId);
    Q_INVOKABLE void dispatchStop(const QString &playerId);
    Q_INVOKABLE void dispatchAssignReplay(const QString &playerId);
    Q_INVOKABLE void dispatchSaveReplay();
    Q_INVOKABLE void dispatchMakePermanent(const QString &playerId, const QString &customFileName = "");

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
    void handleMakePermanent(const QString &playerId, const QString &customFileName);
};

#endif // ACTIONMANAGER_H
