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

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = nullptr);

    void load();
    void save();

    // Getters
    QList<AudioSource> sources() const { return m_sources; }
    bool replayEnabled() const { return m_replayEnabled; }
    int replayDuration() const { return m_replayDuration; }
    QString saveDirectory() const { return m_saveDirectory; }

    // Setters
    void setSources(const QList<AudioSource> &sources) { m_sources = sources; }
    void setReplayEnabled(bool enabled) { m_replayEnabled = enabled; }
    void setReplayDuration(int duration) { m_replayDuration = duration; }
    void setSaveDirectory(const QString &dir) { m_saveDirectory = dir; }

private:
    QString getSettingsFilePath() const;

    QList<AudioSource> m_sources;
    bool m_replayEnabled;
    int m_replayDuration;
    QString m_saveDirectory;
};

#endif // SETTINGSMANAGER_H
