#include "managers/actionmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/recordingmanager.h"
#include "managers/settingsmanager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

void verifyActionManager() {
    qDebug() << "Starting ActionManager verification...";

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

    qDebug() << "Play Dispatched:" << playEmitted;
    qDebug() << "Stop Dispatched:" << stopEmitted;
    qDebug() << "Assign Replay Dispatched:" << assignEmitted;
    qDebug() << "Save Replay Dispatched:" << saveEmitted;

    if (playEmitted && stopEmitted && assignEmitted && saveEmitted) {
        qDebug() << "Verification SUCCESS: All actions dispatched successfully.";
    } else {
        qDebug() << "Verification FAILED.";
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    verifyActionManager();
    return 0;
}
