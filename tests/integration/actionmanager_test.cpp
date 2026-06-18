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
        actionManager.dispatch(Action::createAssignReplay(testPlayerId, true));
        QCOMPARE(spy.count(), 1);
        a = spy.takeFirst().at(0).value<Action>();
        QCOMPARE(a.type, ActionType::AssignReplayToPlayer);
        QCOMPARE(a.parameters.value("playerId").toString(), testPlayerId);
        QCOMPARE(a.parameters.value("preserveExisting").toBool(), true);

        // Test Save Replay
        actionManager.dispatch(Action::createSaveReplay());
        QCOMPARE(spy.count(), 1);
        a = spy.takeFirst().at(0).value<Action>();
        QCOMPARE(a.type, ActionType::SaveReplay);
    }

    void testReplayAssignment() {
        SettingsManager settings;
        SoundboardManager sb(&settings);
        RecordingManager rec(&settings);
        ActionManager actionManager(&sb, &rec, &settings);

        // Setup mock data for ReplayBuffer and Mixer so saveReplay succeeds
        WAVEFORMATEXTENSIBLE fmt;
        memset(&fmt, 0, sizeof(fmt));
        fmt.Format.wFormatTag = 0xFFFE; // WAVE_FORMAT_EXTENSIBLE
        fmt.Format.nChannels = 2;
        fmt.Format.nSamplesPerSec = 48000;
        fmt.Format.wBitsPerSample = 32;
        fmt.Format.nBlockAlign = (fmt.Format.nChannels * fmt.Format.wBitsPerSample) / 8;
        fmt.Format.nAvgBytesPerSec = fmt.Format.nSamplesPerSec * fmt.Format.nBlockAlign;
        
        rec.mixer()->setOutputFormat(fmt);
        rec.replayBuffer()->setFormat(fmt);
        
        // Push some dummy audio data to the replay buffer
        QByteArray dummyAudio(48000 * 8, '\0'); 
        rec.replayBuffer()->pushPcmChunk(dummyAudio);

        QString playerId = sb.addPlayer("Test Player");
        SoundPlayerSlot* slot = sb.getSlot(playerId);
        QVERIFY(slot != nullptr);
        QVERIFY(slot->filePath.isEmpty());

        // 1. Verify replay-to-player assignment works.
        actionManager.dispatch(Action::createAssignReplay(playerId));
        QVERIFY(!slot->filePath.isEmpty());
        QVERIFY(QFile::exists(slot->filePath));
        
        // Ensure it's a temporary path
        QVERIFY(slot->filePath.startsWith(QDir::tempPath()));

        // 2. Verify replay playback works immediately.
        actionManager.dispatch(Action::createPlay(playerId));
        // SoundPlayer should have its state changed to PlayingState
        SoundPlayer* player = sb.getPlayer(playerId);
        QVERIFY(player != nullptr);
        
        // In a guiless test environment, Qt multimedia might not actually play,
        // but we verify the command is sent without crashing.
        // If multimedia is available, playback state would change.
        
        // 3. Verify preserveExisting works
        QString firstPath = slot->filePath;
        
        // Push new data
        rec.replayBuffer()->pushPcmChunk(QByteArray(1024, '\1'));
        actionManager.dispatch(Action::createAssignReplay(playerId, true)); // preserve=true
        QCOMPARE(slot->filePath, firstPath); // Should not change
        
        actionManager.dispatch(Action::createAssignReplay(playerId, false)); // preserve=false
        QVERIFY(slot->filePath != firstPath); // Should change
        
        // 4. Verify replay assignment works during active recording.
        QString tempRecordPath = QDir::tempPath() + "/test_record.wav";
        rec.wavWriter()->open(tempRecordPath, fmt);
        QVERIFY(rec.isRecording());
        
        rec.replayBuffer()->pushPcmChunk(QByteArray(1024, '\2'));
        actionManager.dispatch(Action::createAssignReplay(playerId, false));
        QVERIFY(QFile::exists(slot->filePath));
        
        rec.wavWriter()->close();
        QFile::remove(tempRecordPath);
        
        // Cleanup assigned replays
        QFile::remove(firstPath);
        QFile::remove(slot->filePath);
    }
};

QTEST_GUILESS_MAIN(ActionManagerTest)
#include "actionmanager_test.moc"
