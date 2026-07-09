#include <QtTest>
#include "models/soundplayerslot.h"
#include "managers/settingsmanager.h"
#include "audio/soundplayer.h"

class PlaybackModeUnitTest : public QObject
{
    Q_OBJECT

private slots:
    void testSlotSerialization();
    void testSettingsDefaultMode();
    void testSoundPlayerModeDefaults();
};

void PlaybackModeUnitTest::testSlotSerialization()
{
    SoundPlayerSlot slot;
    QCOMPARE(slot.playbackMode, PlaybackMode::Default);

    slot.playbackMode = PlaybackMode::QueuedSequential;
    QJsonObject json = slot.toJson();
    QCOMPARE(json["playbackMode"].toString(), QString("QueuedSequential"));

    SoundPlayerSlot loaded = SoundPlayerSlot::fromJson(json);
    QCOMPARE(loaded.playbackMode, PlaybackMode::QueuedSequential);

    slot.playbackMode = PlaybackMode::LayeredCutAll;
    json = slot.toJson();
    QCOMPARE(json["playbackMode"].toString(), QString("LayeredCutAll"));

    loaded = SoundPlayerSlot::fromJson(json);
    QCOMPARE(loaded.playbackMode, PlaybackMode::LayeredCutAll);

    slot.playbackMode = PlaybackMode::LayeredRingOut;
    json = slot.toJson();
    QCOMPARE(json["playbackMode"].toString(), QString("LayeredRingOut"));

    loaded = SoundPlayerSlot::fromJson(json);
    QCOMPARE(loaded.playbackMode, PlaybackMode::LayeredRingOut);
}

void PlaybackModeUnitTest::testSettingsDefaultMode()
{
    SettingsManager settings;
    QCOMPARE(settings.defaultPlaybackMode(), PlaybackMode::RestartRetrigger);

    settings.setDefaultPlaybackMode(PlaybackMode::ToggleStop);
    QCOMPARE(settings.defaultPlaybackMode(), PlaybackMode::ToggleStop);

    settings.setDefaultPlaybackMode(PlaybackMode::LayeredCutAll);
    QCOMPARE(settings.defaultPlaybackMode(), PlaybackMode::LayeredCutAll);

    settings.setDefaultPlaybackMode(PlaybackMode::LayeredRingOut);
    QCOMPARE(settings.defaultPlaybackMode(), PlaybackMode::LayeredRingOut);

    // Setting Default to Default falls back to RestartRetrigger
    settings.setDefaultPlaybackMode(PlaybackMode::Default);
    QCOMPARE(settings.defaultPlaybackMode(), PlaybackMode::RestartRetrigger);
}

void PlaybackModeUnitTest::testSoundPlayerModeDefaults()
{
    SoundPlayer player;
    QCOMPARE(player.playbackMode(), PlaybackMode::RestartRetrigger);

    player.setPlaybackMode(PlaybackMode::QueuedSequential);
    QCOMPARE(player.playbackMode(), PlaybackMode::QueuedSequential);

    player.setPlaybackMode(PlaybackMode::LayeredCutAll);
    QCOMPARE(player.playbackMode(), PlaybackMode::LayeredCutAll);

    player.setPlaybackMode(PlaybackMode::LayeredRingOut);
    QCOMPARE(player.playbackMode(), PlaybackMode::LayeredRingOut);
}

QTEST_MAIN(PlaybackModeUnitTest)
#include "playbackmode_test.moc"
