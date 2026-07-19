#include <QtTest>
#include <QSignalSpy>
#include "managers/notificationmanager.h"
#include "managers/settingsmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/recordingmanager.h"

class NotificationManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testSettingsDelegation();
    void testSizeAndPositionDelegation();
    void testPostNotification();
    void testCustomDurationAndSourceId();
    void testGlobalDisable();
    void testPlaybackUpdatedSignal();
};

void NotificationManagerTest::initTestCase()
{
}

void NotificationManagerTest::testSettingsDelegation()
{
    SettingsManager settings;
    NotificationManager notifications(&settings, nullptr, nullptr);

    // Initial default values
    QCOMPARE(notifications.enabled(), true);
    QCOMPARE(notifications.overlayEnabled(), true);
    QCOMPARE(notifications.durationMs(), 3000);

    // Test modifying through NotificationManager
    notifications.setEnabled(false);
    QCOMPARE(settings.notificationsEnabled(), false);
    QCOMPARE(notifications.enabled(), false);

    notifications.setOverlayEnabled(false);
    QCOMPARE(settings.notificationsOverlayEnabled(), false);
    QCOMPARE(notifications.overlayEnabled(), false);

    notifications.setDurationMs(5000);
    QCOMPARE(settings.notificationsDurationMs(), 5000);
    QCOMPARE(notifications.durationMs(), 5000);
}

void NotificationManagerTest::testSizeAndPositionDelegation()
{
    SettingsManager settings;
    NotificationManager notifications(&settings, nullptr, nullptr);

    QCOMPARE(notifications.size(), QString("Medium"));
    QCOMPARE(notifications.position(), QString("BottomRight"));

    QSignalSpy sizeSpy(&notifications, &NotificationManager::sizeChanged);
    QSignalSpy positionSpy(&notifications, &NotificationManager::positionChanged);

    // Test setting size through NotificationManager
    notifications.setSize("Large");
    QCOMPARE(settings.notificationsSize(), QString("Large"));
    QCOMPARE(notifications.size(), QString("Large"));
    QCOMPARE(sizeSpy.count(), 1);

    // Test setting position through NotificationManager
    notifications.setPosition("TopLeft");
    QCOMPARE(settings.notificationsPosition(), QString("TopLeft"));
    QCOMPARE(notifications.position(), QString("TopLeft"));
    QCOMPARE(positionSpy.count(), 1);

    // Test setting through SettingsManager directly
    settings.setNotificationsSize("ExtraSmall");
    QCOMPARE(notifications.size(), QString("ExtraSmall"));
    QCOMPARE(sizeSpy.count(), 2);

    settings.setNotificationsPosition("BottomLeft");
    QCOMPARE(notifications.position(), QString("BottomLeft"));
    QCOMPARE(positionSpy.count(), 2);
}

void NotificationManagerTest::testPostNotification()
{
    SettingsManager settings;
    NotificationManager notifications(&settings, nullptr, nullptr);

    QSignalSpy spy(&notifications, &NotificationManager::notificationPosted);

    notifications.postNotification("Test Title", "Test Message", "play");
    QCOMPARE(spy.count(), 1);

    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), QString("Test Title"));
    QCOMPARE(args.at(1).toString(), QString("Test Message"));
    QCOMPARE(args.at(2).toString(), QString("play"));
    QCOMPARE(args.at(3).toInt(), 3000);
    QCOMPARE(args.at(4).toString(), QString()); // sourceId default is empty
    QCOMPARE(args.at(5).toBool(), false);        // stackDuration default is false
    QCOMPARE(args.at(6).toString(), QString()); // playbackMode default is empty
}

void NotificationManagerTest::testCustomDurationAndSourceId()
{
    SettingsManager settings;
    NotificationManager notifications(&settings, nullptr, nullptr);

    QSignalSpy spy(&notifications, &NotificationManager::notificationPosted);

    // Post with custom duration, sourceId, stackDuration, and playbackMode
    notifications.postNotification("Custom Title", "Custom Msg", "square", 5000, "slot_123", true, "QueuedSequential");
    QCOMPARE(spy.count(), 1);

    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), QString("Custom Title"));
    QCOMPARE(args.at(1).toString(), QString("Custom Msg"));
    QCOMPARE(args.at(2).toString(), QString("square"));
    QCOMPARE(args.at(3).toInt(), 5000);
    QCOMPARE(args.at(4).toString(), QString("slot_123"));
    QCOMPARE(args.at(5).toBool(), true);
    QCOMPARE(args.at(6).toString(), QString("QueuedSequential"));

    // Verify notificationCollapsed signal
    QSignalSpy collapseSpy(&notifications, &NotificationManager::notificationCollapsed);
    emit notifications.notificationCollapsed("slot_123");
    QCOMPARE(collapseSpy.count(), 1);
    QCOMPARE(collapseSpy.takeFirst().at(0).toString(), QString("slot_123"));
}

void NotificationManagerTest::testGlobalDisable()
{
    SettingsManager settings;
    NotificationManager notifications(&settings, nullptr, nullptr);

    QSignalSpy spy(&notifications, &NotificationManager::notificationPosted);

    notifications.setEnabled(false);
    notifications.postNotification("Test Title", "Test Message", "play");

    // Signal should not be emitted when disabled
    QCOMPARE(spy.count(), 0);
}

void NotificationManagerTest::testPlaybackUpdatedSignal()
{
    SettingsManager settings;
    SoundboardManager soundboard(&settings);
    NotificationManager notifications(&settings, &soundboard, nullptr);

    QSignalSpy spy(&notifications, &NotificationManager::notificationPlaybackUpdated);

    // Add a test player slot
    QString slotId = soundboard.addPlayer("Test Player");
    QVERIFY(!slotId.isEmpty());

    // Trigger queue count changed signal on soundboard
    emit soundboard.playerQueueCountChanged(slotId, 3);

    // Verify that notificationPlaybackUpdated was emitted
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), slotId);
    QCOMPARE(args.at(1).toInt(), 3);
    // Since no media is actually loaded in this test player, the remainingMs should fall back to 0 (unknown)
    QCOMPARE(args.at(2).toInt(), 0);
}

QTEST_MAIN(NotificationManagerTest)
#include "notificationmanager_test.moc"
