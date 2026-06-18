#include <QtTest>
#include "managers/soundboardmanager.h"
#include "managers/settingsmanager.h"
#include "audio/soundplayer.h"
#include "audio/waveformgenerator.h"

class AudioClippingTest : public QObject
{
    Q_OBJECT

private slots:
    void testClippingAndSerialization() {
        SettingsManager settings;
        SoundboardManager manager(&settings);

        // Part 1: Default ranges
        QString id = manager.addPlayer("Clipped Player");
        SoundPlayerSlot* slot = manager.getSlot(id);
        QVERIFY(slot != nullptr);
        QCOMPARE(slot->startTimeMs, 0LL);
        QCOMPARE(slot->endTimeMs, -1LL);

        // Update range
        manager.setPlayerClipRange(id, 1000, 5000);
        QCOMPARE(slot->startTimeMs, 1000LL);
        QCOMPARE(slot->endTimeMs, 5000LL);

        SoundPlayer* player = manager.getPlayer(id);
        QVERIFY(player != nullptr);
        QCOMPARE(player->startTimeMs(), 1000LL);
        QCOMPARE(player->endTimeMs(), 5000LL);

        // Save and verify persistence
        manager.saveToSettings();
        
        SettingsManager settingsLoad;
        settingsLoad.load();
        QCOMPARE(settingsLoad.soundBoardSlots().size(), 1);
        QCOMPARE(settingsLoad.soundBoardSlots().at(0).startTimeMs, 1000LL);
        QCOMPARE(settingsLoad.soundBoardSlots().at(0).endTimeMs, 5000LL);
    }

    void testWaveformDummyGeneration() {
        // Since we test in GUI-less / build environment, test generator dummy output
        WaveformData data = WaveformGenerator::generateDummyWaveform(10000, 48000, 2, 128);
        QVERIFY(data.isValid);
        QCOMPARE(data.resolution, 128);
        QCOMPARE(data.durationMs, 10000LL);
        QCOMPARE(data.sampleRate, 48000);
        QCOMPARE(data.channels, 2);
        QCOMPARE(data.peaks.size(), 128);
        
        // Verify peaks are in range [0, 1]
        for (float peak : data.peaks) {
            QVERIFY(peak >= 0.0f && peak <= 1.0f);
        }
    }
};

QTEST_GUILESS_MAIN(AudioClippingTest)
#include "audioclipping_test.moc"
