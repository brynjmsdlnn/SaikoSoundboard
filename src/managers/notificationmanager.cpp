#include "notificationmanager.h"
#include "logging/LogMacros.h"
#include "managers/settingsmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/recordingmanager.h"
#include <QFileInfo>

NotificationManager::NotificationManager(SettingsManager *settings, 
                                         SoundboardManager *soundboard, 
                                         RecordingManager *recording, 
                                         QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_soundboard(soundboard)
    , m_recording(recording)
{
    LOG_INFO(LogCategory::General,
             QStringLiteral("[NotificationManager] Initializing notification subsystem"));

    // Forward settings signals
    connect(m_settings, &SettingsManager::notificationsEnabledChanged, this, &NotificationManager::enabledChanged);
    connect(m_settings, &SettingsManager::notificationsOverlayEnabledChanged, this, &NotificationManager::overlayEnabledChanged);
    connect(m_settings, &SettingsManager::notificationsDurationMsChanged, this, &NotificationManager::durationMsChanged);
    connect(m_settings, &SettingsManager::notificationsSizeChanged, this, &NotificationManager::sizeChanged);
    connect(m_settings, &SettingsManager::notificationsPositionChanged, this, &NotificationManager::positionChanged);

    // Auto-notifications on playback
    if (m_soundboard) {
        connect(m_soundboard, &SoundboardManager::playerPlayStateChanged, this, [this](const QString &id, PlayState state) {
            if (state == PlayState::Playing) {
                SoundPlayerSlot *slot = m_soundboard->getSlot(id);
                if (slot) {
                    QString details = slot->playHotkey.isEmpty() ? QString() : QStringLiteral("Hotkey: %1").arg(slot->playHotkey);
                    LOG_DEBUG(LogCategory::General,
                              QStringLiteral("[NotificationManager] Sound playback auto-notification triggered (name: \"%1\")").arg(slot->name));

                    // Use the same duration source as the grid card: WaveformData + clip range
                    int notificationDuration = durationMs();
                    WaveformData wf = m_soundboard->getWaveformData(id);
                    if (wf.isValid) {
                        qint64 startMs = slot->startTimeMs;
                        qint64 endMs = slot->endTimeMs == -1 ? wf.durationMs : slot->endTimeMs;
                        if (endMs > startMs) {
                            notificationDuration = static_cast<int>(endMs - startMs);
                        }
                    }

                    // In QueuedSequential mode, stack durations cumulatively;
                    // in layered/overlapping modes, reset duration on each play
                    bool stackDuration = (slot->playbackMode == PlaybackMode::QueuedSequential);
                    postNotification(slot->name, details, QStringLiteral("play"), notificationDuration, id, stackDuration);
                }
            } else if (state == PlayState::Stopped) {
                // Instantly collapse the notification for this slot
                emit notificationCollapsed(id);
            }
        });

        // Forward queue count changes for QueuedSequential badge tracking
        connect(m_soundboard, &SoundboardManager::playerQueueCountChanged, this, [this](const QString &id, int count) {
            emit notificationQueueCountChanged(id, count);
        });

        // Slot assignment notifications — from either recording dialog or soundboard file picker
        connect(m_soundboard, &SoundboardManager::audioFileAssigned, this, [this](const QString &slotName, const QString &filePath) {
            Q_UNUSED(filePath)
            LOG_DEBUG(LogCategory::General,
                      QStringLiteral("[NotificationManager] Audio file assigned notification (slot: \"%1\")").arg(slotName));
            postNotification(QStringLiteral("Recording Saved"),
                             QStringLiteral("Assigned to %1").arg(slotName),
                             QStringLiteral("save"));
        });
    }

    // Auto-notifications on recording / replay actions
    if (m_recording) {
        connect(m_recording, &RecordingManager::replaySaved, this, [this](const QString &path) {
            QString name = QFileInfo(path).fileName();
            LOG_DEBUG(LogCategory::General,
                      QStringLiteral("[NotificationManager] Replay saved auto-notification triggered (file: \"%1\")").arg(name));
            postNotification(QStringLiteral("Replay Saved"), name, QStringLiteral("save"));
        });

        connect(m_recording, &RecordingManager::recordingStarted, this, [this](const QString &path) {
            QString name = QFileInfo(path).fileName();
            LOG_DEBUG(LogCategory::General,
                      QStringLiteral("[NotificationManager] Recording started auto-notification (file: \"%1\")").arg(name));
            postNotification(QStringLiteral("Recording Started"), name, QStringLiteral("circle"));
        });

        connect(m_recording, &RecordingManager::recordingStopped, this, [this](const QString &path) {
            QString name = QFileInfo(path).fileName();
            LOG_DEBUG(LogCategory::General,
                      QStringLiteral("[NotificationManager] Recording finished auto-notification (file: \"%1\")").arg(name));
            // Use "Recording Stopped" with square icon since saving is deferred to the dialog
            postNotification(QStringLiteral("Recording Stopped"), name, QStringLiteral("square"));
        });
    }
}

NotificationManager::~NotificationManager()
{
    LOG_INFO(LogCategory::General,
             QStringLiteral("[NotificationManager] Destroying notification subsystem"));
}

bool NotificationManager::enabled() const
{
    return m_settings->notificationsEnabled();
}

void NotificationManager::setEnabled(bool enabled)
{
    m_settings->setNotificationsEnabled(enabled);
}

bool NotificationManager::overlayEnabled() const
{
    return m_settings->notificationsOverlayEnabled();
}

void NotificationManager::setOverlayEnabled(bool enabled)
{
    m_settings->setNotificationsOverlayEnabled(enabled);
}

int NotificationManager::durationMs() const
{
    return m_settings->notificationsDurationMs();
}

void NotificationManager::setDurationMs(int ms)
{
    m_settings->setNotificationsDurationMs(ms);
}

QString NotificationManager::size() const
{
    return m_settings->notificationsSize();
}

void NotificationManager::setSize(const QString &size)
{
    m_settings->setNotificationsSize(size);
}

QString NotificationManager::position() const
{
    return m_settings->notificationsPosition();
}

void NotificationManager::setPosition(const QString &position)
{
    m_settings->setNotificationsPosition(position);
}

void NotificationManager::postNotification(const QString &title, const QString &message, const QString &icon, int customDurationMs, const QString &sourceId, bool stackDuration)
{
    if (!enabled()) {
        LOG_DEBUG(LogCategory::General,
                  QStringLiteral("[NotificationManager] Notification suppressed because notifications are globally disabled"));
        return;
    }

    int duration = (customDurationMs > 0) ? customDurationMs : durationMs();
    LOG_INFO(LogCategory::General,
             QStringLiteral("[NotificationManager] Posting notification (title: \"%1\", message: \"%2\", icon: \"%3\", duration: %4 ms, sourceId: \"%5\", stackDuration: %6)")
                 .arg(title)
                 .arg(message)
                 .arg(icon)
                 .arg(duration)
                 .arg(sourceId)
                 .arg(stackDuration));

    emit notificationPosted(title, message, icon, duration, sourceId, stackDuration);
}
