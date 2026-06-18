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
public:
    explicit SettingsManager(QObject *parent = nullptr);

    void load();
    void save();

    // Getters
    QList<AudioSource> sources() const { return m_sources; }
    QList<SoundPlayerSlot> soundBoardSlots() const { return m_soundBoardSlots; }
    bool replayEnabled() const { return m_replayEnabled; }
    int replayDuration() const { return m_replayDuration; }
    QString saveDirectory() const { return m_saveDirectory; }
    bool enableMicOutput() const { return m_enableMicOutput; }
    bool enableLocalMonitoring() const { return m_enableLocalMonitoring; }
    QString micOutputDevice() const { return m_micOutputDevice; }
    QString localMonitorDevice() const { return m_localMonitorDevice; }

    // Setters
    void setSources(const QList<AudioSource> &sources) { m_sources = sources; }
    void setSoundBoardSlots(const QList<SoundPlayerSlot> &soundBoardSlots) { m_soundBoardSlots = soundBoardSlots; }
    void setReplayEnabled(bool enabled) { m_replayEnabled = enabled; }
    void setReplayDuration(int duration) { m_replayDuration = duration; }
    void setSaveDirectory(const QString &dir) { m_saveDirectory = dir; }
    void setEnableMicOutput(bool enabled) { m_enableMicOutput = enabled; }
    void setEnableLocalMonitoring(bool enabled) { m_enableLocalMonitoring = enabled; }
    void setMicOutputDevice(const QString &device) { m_micOutputDevice = device; }
    void setLocalMonitorDevice(const QString &device) { m_localMonitorDevice = device; }

private:
    QString getSettingsFilePath() const;

    QList<AudioSource> m_sources;
    QList<SoundPlayerSlot> m_soundBoardSlots;
    bool m_replayEnabled;
    int m_replayDuration;
    QString m_saveDirectory;
    bool m_enableMicOutput = true;
    bool m_enableLocalMonitoring = true;
    QString m_micOutputDevice;
    QString m_localMonitorDevice;
};

#endif // SETTINGSMANAGER_H
