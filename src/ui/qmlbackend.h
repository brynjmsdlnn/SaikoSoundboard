#ifndef QMLBACKEND_H
#define QMLBACKEND_H

#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include "managers/settingsmanager.h"
#include "managers/recordingmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/actionmanager.h"
#include "managers/hotkeymanager.h"
#include "models/capturestate.h"

class QmlBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CaptureState captureState READ captureState NOTIFY captureStateChanged)
    Q_PROPERTY(QQmlEngine* engine READ engine CONSTANT)
    Q_PROPERTY(SettingsManager* settings READ settings CONSTANT)
    Q_PROPERTY(RecordingManager* recording READ recordingManager CONSTANT)
    Q_PROPERTY(SoundboardManager* soundboard READ soundboardManager CONSTANT)
    Q_PROPERTY(ActionManager* actions READ actionManager CONSTANT)
    Q_PROPERTY(HotkeyManager* hotkeys READ hotkeyManager CONSTANT)
public:
    explicit QmlBackend(QObject *parent = nullptr);
    ~QmlBackend();

    SettingsManager *settings() const { return m_settings; }
    RecordingManager *recordingManager() const { return m_recordingManager; }
    SoundboardManager *soundboardManager() const { return m_soundboardManager; }
    ActionManager *actionManager() const { return m_actionManager; }
    HotkeyManager *hotkeyManager() const { return m_hotkeyManager; }

    CaptureState captureState() const;
    QQmlEngine *engine() const { return m_engine; }
    QQmlComponent *loadComponent(const QString &qrcPath, QObject *parent = nullptr);

signals:
    void captureStateChanged(CaptureState state);

private slots:
    void reloadComponent(const QString &filePath);

private:
    QQmlEngine *m_engine;
    QFileSystemWatcher *m_qmlWatcher;
    QHash<QString, QString> m_watchedFiles;
    SettingsManager *m_settings;
    RecordingManager *m_recordingManager;
    SoundboardManager *m_soundboardManager;
    ActionManager *m_actionManager;
    HotkeyManager *m_hotkeyManager;
    void *m_hotkeyBackend;
};

#endif
