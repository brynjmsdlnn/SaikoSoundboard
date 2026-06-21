#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QStandardPaths>
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
public:
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

    // Setters
    void setSources(const QList<AudioSource> &sources);
    void setSoundBoardSlots(const QList<SoundPlayerSlot> &soundBoardSlots);
    void setReplayEnabled(bool enabled);
    void setReplayDuration(int duration);
    void setBaseDirectory(const QString &dir);
    void setRecordingDirectoryOverride(const QString &dir);
    void setReplayDirectoryOverride(const QString &dir);
    void setEnableMicOutput(bool enabled);
    void setEnableLocalMonitoring(bool enabled);
    void setMicOutputDevice(const QString &device);
    void setLocalMonitorDevice(const QString &device);
    void setEnableMicPassthrough(bool enabled);
    void setVoiceInputDevice(const QString &device);

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
};

#endif // SETTINGSMANAGER_H
