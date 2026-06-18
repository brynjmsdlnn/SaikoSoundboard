#include <QtTest>
#include "managers/soundboardmanager.h"
#include "managers/settingsmanager.h"
#include "audio/soundplayer.h"

class AudioRoutingTest : public QObject
{
    Q_OBJECT

private slots:
    void testRoutingDecisions() {
        SettingsManager settings;
        SoundboardManager manager(&settings);

        // Part 1: Default routing is Both
        QString idBoth = manager.addPlayer("Both Player");
        SoundPlayer* playerBoth = manager.getPlayer(idBoth);
        QVERIFY(playerBoth != nullptr);
        
        // Confirm default is Both
        QCOMPARE(manager.getSlot(idBoth)->outputRouting, OutputRouting::Both);
        QVERIFY(playerBoth->shouldPlayMic());
        QVERIFY(playerBoth->shouldPlayLocal());

        // Part 2: Mic Only
        QString idMic = manager.addPlayer("Mic Player");
        manager.setPlayerRouting(idMic, OutputRouting::MicOnly);
        SoundPlayer* playerMic = manager.getPlayer(idMic);
        QVERIFY(playerMic != nullptr);
        QVERIFY(playerMic->shouldPlayMic());
        QVERIFY(!playerMic->shouldPlayLocal());

        // Part 3: Local Only
        QString idLocal = manager.addPlayer("Local Player");
        manager.setPlayerRouting(idLocal, OutputRouting::LocalOnly);
        SoundPlayer* playerLocal = manager.getPlayer(idLocal);
        QVERIFY(playerLocal != nullptr);
        QVERIFY(!playerLocal->shouldPlayMic());
        QVERIFY(playerLocal->shouldPlayLocal());

        // Part 6: Global overrides
        // Disable mic output globally
        manager.setMicOutputEnabled(false);
        QVERIFY(!playerBoth->shouldPlayMic());
        QVERIFY(playerBoth->shouldPlayLocal()); // local still works
        QVERIFY(!playerMic->shouldPlayMic());
        QVERIFY(!playerMic->shouldPlayLocal());

        // Enable mic output globally, disable local globally
        manager.setMicOutputEnabled(true);
        manager.setLocalMonitoringEnabled(false);
        QVERIFY(playerBoth->shouldPlayMic());
        QVERIFY(!playerBoth->shouldPlayLocal());
        QVERIFY(!playerLocal->shouldPlayMic());
        QVERIFY(!playerLocal->shouldPlayLocal());

        // Reset global settings
        manager.setLocalMonitoringEnabled(true);
        QVERIFY(playerBoth->shouldPlayMic());
        QVERIFY(playerBoth->shouldPlayLocal());
    }

    void testPersistence() {
        // Create temporary settings
        SettingsManager settings;
        // Set master settings
        settings.setEnableMicOutput(false);
        settings.setEnableLocalMonitoring(true);
        settings.setMicOutputDevice("Virtual Mic");
        settings.setLocalMonitorDevice("Headphones");

        // Add slots with various routing
        QList<SoundPlayerSlot> slotList;
        SoundPlayerSlot slot1;
        slot1.name = "Test 1";
        slot1.outputRouting = OutputRouting::MicOnly;
        slotList.append(slot1);

        SoundPlayerSlot slot2;
        slot2.name = "Test 2";
        slot2.outputRouting = OutputRouting::LocalOnly;
        slotList.append(slot2);

        settings.setSoundBoardSlots(slotList);
        settings.save();

        // Load settings in a new SettingsManager instance
        SettingsManager settingsLoad;
        settingsLoad.load();

        QCOMPARE(settingsLoad.enableMicOutput(), false);
        QCOMPARE(settingsLoad.enableLocalMonitoring(), true);
        QCOMPARE(settingsLoad.micOutputDevice(), QString("Virtual Mic"));
        QCOMPARE(settingsLoad.localMonitorDevice(), QString("Headphones"));

        QCOMPARE(settingsLoad.soundBoardSlots().size(), 2);
        QCOMPARE(settingsLoad.soundBoardSlots().at(0).outputRouting, OutputRouting::MicOnly);
        QCOMPARE(settingsLoad.soundBoardSlots().at(1).outputRouting, OutputRouting::LocalOnly);
    }
};

QTEST_GUILESS_MAIN(AudioRoutingTest)
#include "audiorouting_test.moc"
