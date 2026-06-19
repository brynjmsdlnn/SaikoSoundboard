#ifndef QMLBACKEND_H
#define QMLBACKEND_H

#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickImageProvider>
#include <QFileIconProvider>
#include <QIcon>
#include <QPixmap>
#include <QFileInfo>
#include <QUrl>
#include "managers/settingsmanager.h"
#include "managers/recordingmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/actionmanager.h"
#include "managers/hotkeymanager.h"
#include "models/capturestate.h"
#include "models/soundplayerslotmodel.h"

class FileIconProvider : public QQuickImageProvider
{
public:
    FileIconProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        QString filePath = QUrl::fromPercentEncoding(id.toUtf8());
        QFileIconProvider provider;
        QIcon icon = provider.icon(QFileInfo(filePath));
        QSize actualSize = requestedSize.isValid() ? requestedSize : QSize(32, 32);
        if (size) *size = actualSize;
        return icon.pixmap(actualSize);
    }
};

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
    Q_PROPERTY(SoundPlayerSlotModel* slotModel READ slotModel CONSTANT)
public:
    explicit QmlBackend(QObject *parent = nullptr);
    ~QmlBackend();

    SettingsManager *settings() const { return m_settings; }
    RecordingManager *recordingManager() const { return m_recordingManager; }
    SoundboardManager *soundboardManager() const { return m_soundboardManager; }
    ActionManager *actionManager() const { return m_actionManager; }
    HotkeyManager *hotkeyManager() const { return m_hotkeyManager; }
    SoundPlayerSlotModel *slotModel() const { return m_slotModel; }

    CaptureState captureState() const;
    QQmlEngine *engine() const { return m_engine; }
    Q_INVOKABLE QVariantList getRunningProcesses() const;
    Q_INVOKABLE QVariantList getAudioOutputDevices() const;
    Q_INVOKABLE QVariantList getAudioInputDevices() const;
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
    SoundPlayerSlotModel *m_slotModel = nullptr;
    void *m_hotkeyBackend;
};

#endif
