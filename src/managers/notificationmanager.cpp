#include "notificationmanager.h"
#include "logging/LogMacros.h"
#include "managers/settingsmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/recordingmanager.h"
#include "models/soundplayerslot.h"
#include "storage/StoragePaths.h"

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
        // Forward player stop reasons to the QML layer for two-phase lifecycle
        connect(m_soundboard, &SoundboardManager::playerStopped, this, [this](const QString &id, StopReason reason) {
            QString reasonStr;
            switch (reason) {
                case StopReason::Natural:     reasonStr = QStringLiteral("Natural"); break;
                case StopReason::User:        reasonStr = QStringLiteral("User"); break;
                case StopReason::Interrupted: reasonStr = QStringLiteral("Interrupted"); break;
                case StopReason::Error:       reasonStr = QStringLiteral("Error"); break;
            }
            emit notificationPlaybackStopped(id, reasonStr);
        });

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

                    // Resolve the effective playback mode
                    PlaybackMode effectiveMode = slot->playbackMode;
                    if (effectiveMode == PlaybackMode::Default) {
                        effectiveMode = m_settings->defaultPlaybackMode();
                    }
                    bool stackDuration = (effectiveMode == PlaybackMode::QueuedSequential);

                    // Extend details with playback mode status indicator
                    QString extendedDetails = details;
                    QString modeStr = playbackModeToString(effectiveMode);
                    if (!extendedDetails.isEmpty()) {
                        extendedDetails += QStringLiteral(" \u2022 ");
                    }
                    // Human-friendly mode label for the subtitle
                    switch (effectiveMode) {
                        case PlaybackMode::QueuedSequential:
                            extendedDetails += QStringLiteral("Queued");
                            break;
                        case PlaybackMode::ToggleStop:
                            extendedDetails += QStringLiteral("Press to Stop");
                            break;
                        case PlaybackMode::LayeredCutAll:
                            extendedDetails += QStringLiteral("Overlaps, Cuts Others");
                            break;
                        case PlaybackMode::LayeredRingOut:
                            extendedDetails += QStringLiteral("Overlaps, Rings Out");
                            break;
                        default:
                            extendedDetails += QStringLiteral("Restarts");
                            break;
                    }

                    postNotification(slot->name, extendedDetails, QStringLiteral("play"), notificationDuration, id, stackDuration, modeStr);
                }                }
        });

        // Forward active voice count for LED indicators in notifications
        connect(m_soundboard, &SoundboardManager::playerActiveVoiceCountChanged, this, [this](const QString &id, int count) {
            emit notificationActiveVoiceCountChanged(id, count);
        });

        // Forward queue count and remaining time updates for QueuedSequential badge and timeline tracking
        connect(m_soundboard, &SoundboardManager::playerQueueCountChanged, [this](const QString &id, int count) {
            int remainingMs = static_cast<int>(m_soundboard->getPlayerRemainingPlayTimeMs(id));
            emit notificationPlaybackUpdated(id, count, remainingMs);
        });

        // Slot assignment notifications — from either replay assignment, recording dialog, or file picker
        connect(m_soundboard, &SoundboardManager::audioFileAssigned, this, [this](const QString &slotName, const QString &filePath) {
            QString title;
            if (StoragePaths::isTemporaryPath(filePath)) {
                // Instant replay assignment (temp file was just saved and assigned in one action)
                title = QStringLiteral("Replay Saved & Assigned");
            } else {
                // Standard slot assignment (from recording dialog, file picker, or make-permanent)
                title = QStringLiteral("Slot Assigned");
            }
            LOG_DEBUG(LogCategory::General,
                      QStringLiteral("[NotificationManager] Audio file assigned notification (slot: \"%1\", title: \"%2\")").arg(slotName, title));
            postNotification(title,
                             QStringLiteral("Assigned to %1").arg(slotName),
                             QStringLiteral("save"));
        });
    }

    // Auto-notifications on recording / replay actions
    if (m_recording) {
        connect(m_recording, &RecordingManager::replaySaved, this, [this](const QString &path) {
            // Suppress notification for temporary paths — those are immediately assigned to a slot
            // and will show a unified "Replay Saved & Assigned" notification instead.
            if (StoragePaths::isTemporaryPath(path)) {
                LOG_DEBUG(LogCategory::General,
                          QStringLiteral("[NotificationManager] Suppressing replaySaved notification for temporary path"));
                return;
            }
            // Permanent save (from QML Save button or SaveReplay action)
            LOG_DEBUG(LogCategory::General,
                      QStringLiteral("[NotificationManager] Replay saved auto-notification triggered (permanent path)"));
            postNotification(QStringLiteral("Replay Saved"), QString(), QStringLiteral("save"));
        });

        connect(m_recording, &RecordingManager::recordingStarted, this, [this](const QString &path) {
            Q_UNUSED(path)
            LOG_DEBUG(LogCategory::General,
                      QStringLiteral("[NotificationManager] Recording started auto-notification"));
            postNotification(QStringLiteral("Recording Started"), QString(), QStringLiteral("circle"));
        });

        connect(m_recording, &RecordingManager::recordingStopped, this, [this](const QString &path) {
            Q_UNUSED(path)
            LOG_DEBUG(LogCategory::General,
                      QStringLiteral("[NotificationManager] Recording finished auto-notification"));
            // Use "Recording Stopped" with square icon since saving is deferred to the dialog
            postNotification(QStringLiteral("Recording Stopped"), QString(), QStringLiteral("square"));
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

void NotificationManager::postNotification(const QString &title, const QString &message, const QString &icon, int customDurationMs, const QString &sourceId, bool stackDuration, const QString &playbackMode)
{
    if (!enabled()) {
        LOG_DEBUG(LogCategory::General,
                  QStringLiteral("[NotificationManager] Notification suppressed because notifications are globally disabled"));
        return;
    }

    int duration = (customDurationMs > 0) ? customDurationMs : durationMs();
    LOG_INFO(LogCategory::General,
             QStringLiteral("[NotificationManager] Posting notification (title: \"%1\", message: \"%2\", icon: \"%3\", duration: %4 ms, sourceId: \"%5\", stackDuration: %6, playbackMode: \"%7\")")
                 .arg(title)
                 .arg(message)
                 .arg(icon)
                 .arg(duration)
                 .arg(sourceId)
                 .arg(stackDuration)
                 .arg(playbackMode));

    emit notificationPosted(title, message, icon, duration, sourceId, stackDuration, playbackMode);
}
