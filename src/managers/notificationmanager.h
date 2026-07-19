#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QString>

class SettingsManager;
class SoundboardManager;
class RecordingManager;

class NotificationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool overlayEnabled READ overlayEnabled WRITE setOverlayEnabled NOTIFY overlayEnabledChanged)
    Q_PROPERTY(int durationMs READ durationMs WRITE setDurationMs NOTIFY durationMsChanged)
    Q_PROPERTY(QString size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(QString position READ position WRITE setPosition NOTIFY positionChanged)

public:
    explicit NotificationManager(SettingsManager *settings, 
                                 SoundboardManager *soundboard, 
                                 RecordingManager *recording, 
                                 QObject *parent = nullptr);
    ~NotificationManager();

    bool enabled() const;
    void setEnabled(bool enabled);

    bool overlayEnabled() const;
    void setOverlayEnabled(bool enabled);

    int durationMs() const;
    void setDurationMs(int ms);

    QString size() const;
    void setSize(const QString &size);

    QString position() const;
    void setPosition(const QString &position);

    Q_INVOKABLE void postNotification(const QString &title, const QString &message, const QString &icon = "info", int customDurationMs = -1, const QString &sourceId = QString(), bool stackDuration = false, const QString &playbackMode = QString());

signals:
    void enabledChanged();
    void overlayEnabledChanged();
    void durationMsChanged();
    void sizeChanged();
    void positionChanged();
    void notificationPosted(const QString &title, const QString &message, const QString &icon, int durationMs, const QString &sourceId, bool stackDuration, const QString &playbackMode);
    void notificationCollapsed(const QString &sourceId);
    void notificationQueueCountChanged(const QString &sourceId, int queueCount);

private:
    SettingsManager *m_settings;
    SoundboardManager *m_soundboard;
    RecordingManager *m_recording;
};

#endif // NOTIFICATIONMANAGER_H
