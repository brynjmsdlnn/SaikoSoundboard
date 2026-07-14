#include "managers/actionmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/recordingmanager.h"
#include "managers/settingsmanager.h"
#include "logging/LogMacros.h"
#include <QCoreApplication>
#include <QTimer>

void verifyActionManager() {
    LOG_DEBUG(LogCategory::General, QStringLiteral("Starting ActionManager verification..."));

    SettingsManager settings;
    SoundboardManager sb(&settings);
    RecordingManager rec(&settings);
    ActionManager actionManager(&sb, &rec, &settings);

    bool playEmitted = false;
    bool stopEmitted = false;
    bool assignEmitted = false;
    bool saveEmitted = false;

    QObject::connect(&actionManager, &ActionManager::actionDispatched, [&](const Action &a){
        if (a.type == ActionType::PlayPlayer) playEmitted = true;
        if (a.type == ActionType::StopPlayer) stopEmitted = true;
        if (a.type == ActionType::AssignReplayToPlayer) assignEmitted = true;
        if (a.type == ActionType::SaveReplay) saveEmitted = true;
    });

    QString testPlayerId = "test_player_123";

    actionManager.dispatch(Action::createPlay(testPlayerId));
    actionManager.dispatch(Action::createStop(testPlayerId));
    actionManager.dispatch(Action::createAssignReplay(testPlayerId));
    actionManager.dispatch(Action::createSaveReplay());

    LOG_DEBUG(LogCategory::General, QStringLiteral("Play Dispatched: %1").arg(playEmitted));
    LOG_DEBUG(LogCategory::General, QStringLiteral("Stop Dispatched: %1").arg(stopEmitted));
    LOG_DEBUG(LogCategory::General, QStringLiteral("Assign Replay Dispatched: %1").arg(assignEmitted));
    LOG_DEBUG(LogCategory::General, QStringLiteral("Save Replay Dispatched: %1").arg(saveEmitted));

    if (playEmitted && stopEmitted && assignEmitted && saveEmitted) {
        LOG_DEBUG(LogCategory::General, QStringLiteral("Verification SUCCESS: All actions dispatched successfully."));
    } else {
        LOG_DEBUG(LogCategory::General, QStringLiteral("Verification FAILED."));
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    verifyActionManager();
    return 0;
}
