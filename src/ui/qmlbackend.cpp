#include "ui/qmlbackend.h"
#include <QFileInfo>
#include <QQmlContext>
#include <QUrl>
#include "core/adapters/WindowsHotkeyBackend.h"

QmlBackend::QmlBackend(QObject *parent)
    : QObject(parent)
    , m_engine(new QQmlEngine(this))
    , m_qmlWatcher(new QFileSystemWatcher(this))
{
    connect(m_qmlWatcher, &QFileSystemWatcher::fileChanged, this, &QmlBackend::reloadComponent);
    m_settings = new SettingsManager(this);
    m_settings->load();

    m_recordingManager = new RecordingManager(m_settings, this);

    m_soundboardManager = new SoundboardManager(m_settings, this);
    m_soundboardManager->loadFromSettings();

    m_actionManager = new ActionManager(m_soundboardManager, m_recordingManager, m_settings, this);

    auto *backend = new Saiko::Adapters::WindowsHotkeyBackend();
    m_hotkeyBackend = backend;
    m_hotkeyManager = new HotkeyManager(m_actionManager, backend, this);

    connect(m_recordingManager, &RecordingManager::stateChanged, this, &QmlBackend::captureStateChanged);

    m_engine->rootContext()->setContextProperty("qmlBackend", this);
}

QmlBackend::~QmlBackend()
{
    m_soundboardManager->saveToSettings();
    m_settings->save();
    delete static_cast<Saiko::Adapters::WindowsHotkeyBackend*>(m_hotkeyBackend);
}

QQmlComponent *QmlBackend::loadComponent(const QString &qrcPath, QObject *parent)
{
    m_engine->clearComponentCache();

#ifdef QT_DEBUG
    QString qmlDir = qEnvironmentVariable("QML_SOURCES_PATH",
        "C:/Users/B/Desktop/Qt Projects/SaikoSoundboard/src/qml");
    QFileInfo fi(qrcPath);
    QString localFile = qmlDir + "/" + fi.fileName();
    m_watchedFiles[localFile] = qrcPath;
    m_qmlWatcher->addPath(localFile);
    return new QQmlComponent(m_engine, QUrl::fromLocalFile(localFile), parent);
#else
    Q_UNUSED(m_qmlWatcher);
    Q_UNUSED(m_watchedFiles);
    return new QQmlComponent(m_engine, QUrl(qrcPath), parent);
#endif
}

void QmlBackend::reloadComponent(const QString &filePath)
{
    m_engine->clearComponentCache();
    if (m_watchedFiles.contains(filePath)) {
        m_qmlWatcher->addPath(filePath);
    }
}

CaptureState QmlBackend::captureState() const
{
    return m_recordingManager ? m_recordingManager->state() : CaptureState::Idle;
}
