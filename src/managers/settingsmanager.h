#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QList>
#include "models/audiosource.h"
#include "models/soundplayerslot.h"

class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<AudioSource> sources READ sources NOTIFY sourcesChanged)
    Q_PROPERTY(QList<SoundPlayerSlot> soundBoardSlots READ soundBoardSlots NOTIFY soundBoardSlotsChanged)
    Q_PROPERTY(bool replayEnabled READ replayEnabled WRITE setReplayEnabled NOTIFY replayEnabledChanged)
    Q_PROPERTY(int replayDuration READ replayDuration WRITE setReplayDuration NOTIFY replayDurationChanged)
    Q_PROPERTY(QString baseDirectory READ baseDirectory WRITE setBaseDirectory NOTIFY baseDirectoryChanged)
    Q_PROPERTY(QString recordingDirectoryOverride READ recordingDirectoryOverride WRITE setRecordingDirectoryOverride NOTIFY recordingDirectoryOverrideChanged)
    Q_PROPERTY(QString replayDirectoryOverride READ replayDirectoryOverride WRITE setReplayDirectoryOverride NOTIFY replayDirectoryOverrideChanged)
    Q_PROPERTY(QString recordingDirectory READ recordingDirectory NOTIFY recordingDirectoryChanged)
    Q_PROPERTY(QString replayDirectory READ replayDirectory NOTIFY replayDirectoryChanged)
    Q_PROPERTY(bool enableMicOutput READ enableMicOutput WRITE setEnableMicOutput NOTIFY enableMicOutputChanged)
    Q_PROPERTY(bool enableLocalMonitoring READ enableLocalMonitoring WRITE setEnableLocalMonitoring NOTIFY enableLocalMonitoringChanged)
    Q_PROPERTY(QString micOutputDevice READ micOutputDevice WRITE setMicOutputDevice NOTIFY micOutputDeviceChanged)
    Q_PROPERTY(QString localMonitorDevice READ localMonitorDevice WRITE setLocalMonitorDevice NOTIFY localMonitorDeviceChanged)
    Q_PROPERTY(bool enableMicPassthrough READ enableMicPassthrough WRITE setEnableMicPassthrough NOTIFY enableMicPassthroughChanged)
    Q_PROPERTY(QString voiceInputDevice READ voiceInputDevice WRITE setVoiceInputDevice NOTIFY voiceInputDeviceChanged)
    Q_PROPERTY(int recordingSampleRate READ recordingSampleRate WRITE setRecordingSampleRate NOTIFY recordingSampleRateChanged)
    Q_PROPERTY(bool hotkeysEnabled READ hotkeysEnabled WRITE setHotkeysEnabled NOTIFY hotkeysEnabledChanged)
    Q_PROPERTY(PlaybackMode defaultPlaybackMode READ defaultPlaybackMode WRITE setDefaultPlaybackMode NOTIFY defaultPlaybackModeChanged)
    Q_PROPERTY(bool notificationsEnabled READ notificationsEnabled WRITE setNotificationsEnabled NOTIFY notificationsEnabledChanged)
    Q_PROPERTY(bool notificationsOverlayEnabled READ notificationsOverlayEnabled WRITE setNotificationsOverlayEnabled NOTIFY notificationsOverlayEnabledChanged)
    Q_PROPERTY(int notificationsDurationMs READ notificationsDurationMs WRITE setNotificationsDurationMs NOTIFY notificationsDurationMsChanged)
    Q_PROPERTY(QString notificationsSize READ notificationsSize WRITE setNotificationsSize NOTIFY notificationsSizeChanged)
    Q_PROPERTY(QString notificationsPosition READ notificationsPosition WRITE setNotificationsPosition NOTIFY notificationsPositionChanged)
    Q_PROPERTY(CloseBehavior closeBehavior READ closeBehavior WRITE setCloseBehavior NOTIFY closeBehaviorChanged)
    Q_PROPERTY(bool hasShownFirstHideNotification READ hasShownFirstHideNotification WRITE setHasShownFirstHideNotification NOTIFY hasShownFirstHideNotificationChanged)
    Q_PROPERTY(QString trayIconColor READ trayIconColor WRITE setTrayIconColor NOTIFY trayIconColorChanged)
public:
    enum class CloseBehavior {
        Ask = 0,
        MinimizeToTray = 1,
        Exit = 2
    };
    Q_ENUM(CloseBehavior)
    explicit SettingsManager(QObject *parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void save();

    // Getters
    QList<AudioSource> sources() const { return m_sources; }
    QList<SoundPlayerSlot> soundBoardSlots() const { return m_soundBoardSlots; }
    bool replayEnabled() const { return m_replayEnabled; }
    int replayDuration() const { return m_replayDuration; }
    QString baseDirectory() const { return m_baseDirectory; }
    QString recordingDirectoryOverride() const { return m_recordingDirectoryOverride; }
    QString replayDirectoryOverride() const { return m_replayDirectoryOverride; }
    QString recordingDirectory() const;
    QString replayDirectory() const;
    bool enableMicOutput() const { return m_enableMicOutput; }
    bool enableLocalMonitoring() const { return m_enableLocalMonitoring; }
    QString micOutputDevice() const { return m_micOutputDevice; }
    QString localMonitorDevice() const { return m_localMonitorDevice; }
    bool enableMicPassthrough() const { return m_enableMicPassthrough; }
    QString voiceInputDevice() const { return m_voiceInputDevice; }
    int recordingSampleRate() const { return m_recordingSampleRate; }
    bool hotkeysEnabled() const { return m_hotkeysEnabled; }
    PlaybackMode defaultPlaybackMode() const { return m_defaultPlaybackMode; }
    bool notificationsEnabled() const { return m_notificationsEnabled; }
    bool notificationsOverlayEnabled() const { return m_notificationsOverlayEnabled; }
    int notificationsDurationMs() const { return m_notificationsDurationMs; }
    QString notificationsSize() const { return m_notificationsSize; }
    QString notificationsPosition() const { return m_notificationsPosition; }
    CloseBehavior closeBehavior() const { return m_closeBehavior; }
    bool hasShownFirstHideNotification() const { return m_hasShownFirstHideNotification; }
    QString trayIconColor() const { return m_trayIconColor; }

    // Setters
    Q_INVOKABLE void setSources(const QList<AudioSource> &sources);
    Q_INVOKABLE void setSoundBoardSlots(const QList<SoundPlayerSlot> &soundBoardSlots);
    Q_INVOKABLE void setReplayEnabled(bool enabled);
    Q_INVOKABLE void setReplayDuration(int duration);
    Q_INVOKABLE void setBaseDirectory(const QString &dir);
    Q_INVOKABLE void setRecordingDirectoryOverride(const QString &dir);
    Q_INVOKABLE void setReplayDirectoryOverride(const QString &dir);
    Q_INVOKABLE void setEnableMicOutput(bool enabled);
    Q_INVOKABLE void setEnableLocalMonitoring(bool enabled);
    Q_INVOKABLE void setMicOutputDevice(const QString &device);
    Q_INVOKABLE void setLocalMonitorDevice(const QString &device);
    Q_INVOKABLE void setEnableMicPassthrough(bool enabled);
    Q_INVOKABLE void setVoiceInputDevice(const QString &device);
    Q_INVOKABLE void setRecordingSampleRate(int sampleRate);
    Q_INVOKABLE void setHotkeysEnabled(bool enabled);
    Q_INVOKABLE void setDefaultPlaybackMode(PlaybackMode mode);
    Q_INVOKABLE void setNotificationsEnabled(bool enabled);
    Q_INVOKABLE void setNotificationsOverlayEnabled(bool enabled);
    Q_INVOKABLE void setNotificationsDurationMs(int durationMs);
    Q_INVOKABLE void setNotificationsSize(const QString &size);
    Q_INVOKABLE void setNotificationsPosition(const QString &position);
    Q_INVOKABLE void setCloseBehavior(CloseBehavior behavior);
    Q_INVOKABLE void setHasShownFirstHideNotification(bool value);
    Q_INVOKABLE void setTrayIconColor(const QString &color);

signals:
    void sourcesChanged();
    void soundBoardSlotsChanged();
    void replayEnabledChanged();
    void replayDurationChanged();
    void baseDirectoryChanged();
    void recordingDirectoryOverrideChanged();
    void replayDirectoryOverrideChanged();
    void recordingDirectoryChanged();
    void replayDirectoryChanged();
    void enableMicOutputChanged();
    void enableLocalMonitoringChanged();
    void micOutputDeviceChanged();
    void localMonitorDeviceChanged();
    void enableMicPassthroughChanged();
    void voiceInputDeviceChanged();
    void recordingSampleRateChanged();
    void hotkeysEnabledChanged();
    void defaultPlaybackModeChanged();
    void notificationsEnabledChanged();
    void notificationsOverlayEnabledChanged();
    void notificationsDurationMsChanged();
    void notificationsSizeChanged();
    void notificationsPositionChanged();
    void closeBehaviorChanged();
    void hasShownFirstHideNotificationChanged();
    void trayIconColorChanged();

private:
    QString getSettingsFilePath() const;

    QList<AudioSource> m_sources;
    QList<SoundPlayerSlot> m_soundBoardSlots;
    bool m_replayEnabled;
    int m_replayDuration;
    QString m_baseDirectory;
    QString m_recordingDirectoryOverride;
    QString m_replayDirectoryOverride;
    bool m_enableMicOutput = true;
    bool m_enableLocalMonitoring = true;
    QString m_micOutputDevice;
    QString m_localMonitorDevice;
    bool m_enableMicPassthrough = false;
    QString m_voiceInputDevice;
    int m_recordingSampleRate = 48000;
    bool m_hotkeysEnabled = true;
    PlaybackMode m_defaultPlaybackMode = PlaybackMode::RestartRetrigger;
    bool m_notificationsEnabled = true;
    bool m_notificationsOverlayEnabled = true;
    int m_notificationsDurationMs = 3000;
    QString m_notificationsSize = QStringLiteral("Medium");
    QString m_notificationsPosition = QStringLiteral("BottomRight");
    CloseBehavior m_closeBehavior = CloseBehavior::Ask;
    bool m_hasShownFirstHideNotification = false;
    QString m_trayIconColor = QStringLiteral("#e35d5d");
};

#endif // SETTINGSMANAGER_H
