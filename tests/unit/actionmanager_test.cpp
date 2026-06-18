#include <QtTest>
#include <QSignalSpy>
#include "managers/actionmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/recordingmanager.h"
#include "managers/settingsmanager.h"

class ActionManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        qRegisterMetaType<Action>("Action");
    }

    void testActionDispatch() {
        SettingsManager settings;
        SoundboardManager sb(&settings);
        RecordingManager rec(&settings);
        ActionManager actionManager(&sb, &rec, &settings);

        QSignalSpy spy(&actionManager, &ActionManager::actionDispatched);

        QString testPlayerId = "test_player_123";

        // Test Play Action
        actionManager.dispatch(Action::createPlay(testPlayerId));
        QCOMPARE(spy.count(), 1);
        Action a = spy.takeFirst().at(0).value<Action>();
        QCOMPARE(a.type, ActionType::PlayPlayer);
        QCOMPARE(a.parameters.value("playerId").toString(), testPlayerId);

        // Test Stop Action
        actionManager.dispatch(Action::createStop(testPlayerId));
        QCOMPARE(spy.count(), 1);
        a = spy.takeFirst().at(0).value<Action>();
        QCOMPARE(a.type, ActionType::StopPlayer);
        QCOMPARE(a.parameters.value("playerId").toString(), testPlayerId);

        // Test Assign Replay
        actionManager.dispatch(Action::createAssignReplay(testPlayerId));
        QCOMPARE(spy.count(), 1);
        a = spy.takeFirst().at(0).value<Action>();
        QCOMPARE(a.type, ActionType::AssignReplayToPlayer);
        QCOMPARE(a.parameters.value("playerId").toString(), testPlayerId);

        // Test Save Replay
        actionManager.dispatch(Action::createSaveReplay());
        QCOMPARE(spy.count(), 1);
        a = spy.takeFirst().at(0).value<Action>();
        QCOMPARE(a.type, ActionType::SaveReplay);
    }
};

QTEST_GUILESS_MAIN(ActionManagerTest)
#include "actionmanager_test.moc"
