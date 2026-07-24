#include "lifecycle/ApplicationLifecycleManager.h"
#include "ui/qmlbackend.h"
#include "logging/LogMacros.h"
#include <QCoreApplication>
#include <QQuickWindow>
#include <QIcon>
#include <QSvgRenderer>
#include <QPainter>
#include <QByteArray>
#include <QDebug>

ApplicationLifecycleManager::ApplicationLifecycleManager(QmlBackend *backend, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
{
    LOG_INFO(LogCategory::General, QStringLiteral("[Lifecycle] ApplicationLifecycleManager initialized"));
    setupTrayIcon();

    // React to tray icon color changes from settings
    if (m_backend && m_backend->settings()) {
        connect(m_backend->settings(), &SettingsManager::trayIconColorChanged,
                this, &ApplicationLifecycleManager::onTrayIconColorChanged);
    }
}

ApplicationLifecycleManager::~ApplicationLifecycleManager()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

ApplicationLifecycleManager::CloseBehavior ApplicationLifecycleManager::closeBehavior() const
{
    if (m_backend && m_backend->settings()) {
        return m_backend->settings()->closeBehavior();
    }
    return CloseBehavior::Ask;
}

void ApplicationLifecycleManager::setCloseBehavior(CloseBehavior behavior)
{
    if (m_backend && m_backend->settings()) {
        if (m_backend->settings()->closeBehavior() != behavior) {
            m_backend->settings()->setCloseBehavior(behavior);
            emit closeBehaviorChanged();
        }
    }
}

void ApplicationLifecycleManager::setState(ApplicationState newState)
{
    if (m_state != newState) {
        m_state = newState;
        LOG_INFO(LogCategory::General,
                 QStringLiteral("[Lifecycle] State changed to: %1")
                     .arg(static_cast<int>(m_state)));
        emit stateChanged();
    }
}

void ApplicationLifecycleManager::setupTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        LOG_WARN(LogCategory::General, QStringLiteral("[Lifecycle] System tray is not available on this system"));
        return;
    }

    m_trayIcon = new QSystemTrayIcon(this);

    // Use saved color or default red
    updateTrayIcon();

    m_trayIcon->setToolTip(QStringLiteral("Saiko Soundboard"));

    m_trayMenu = new QMenu();
    m_openAction = m_trayMenu->addAction(QStringLiteral("Open Saiko Soundboard"), this, &ApplicationLifecycleManager::restoreWindow);
    m_trayMenu->addSeparator();
    m_exitAction = m_trayMenu->addAction(QStringLiteral("Exit"), this, &ApplicationLifecycleManager::exitApplication);

    m_trayIcon->setContextMenu(m_trayMenu);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &ApplicationLifecycleManager::onTrayIconActivated);

    m_trayIcon->show();
    LOG_INFO(LogCategory::General, QStringLiteral("[Lifecycle] System tray icon initialized and shown"));
}

void ApplicationLifecycleManager::updateTrayIcon()
{
    if (!m_trayIcon)
        return;

    QColor color(QStringLiteral("#e35d5d")); // default red
    if (m_backend && m_backend->settings()) {
        color = QColor(m_backend->settings()->trayIconColor());
    }

    m_trayIcon->setIcon(createColoredIcon(color));
    LOG_DEBUG(LogCategory::General,
              QStringLiteral("[Lifecycle] Tray icon color updated to: %1").arg(color.name()));
}

QIcon ApplicationLifecycleManager::createColoredIcon(const QColor &color) const
{
    QFile file(QStringLiteral(":/icons/radio.svg"));
    if (!file.open(QIODevice::ReadOnly)) {
        return QIcon();
    }

    QString svgData = QString::fromUtf8(file.readAll());
    svgData.replace(QStringLiteral("currentColor"), color.name());

    QSvgRenderer renderer(svgData.toUtf8());
    QSize size = renderer.defaultSize();
    if (size.isEmpty())
        size = QSize(24, 24);

    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    renderer.render(&painter);
    painter.end();

    return QPixmap::fromImage(image);
}

void ApplicationLifecycleManager::onTrayIconColorChanged()
{
    updateTrayIcon();
}

void ApplicationLifecycleManager::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (m_state == ApplicationState::ShuttingDown || m_state == ApplicationState::Terminated)
        return;

    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        restoreWindow();
        break;
    default:
        break;
    }
}

void ApplicationLifecycleManager::attachMainWindow(QQuickWindow *window)
{
    if (m_mainWindowAttached) {
        LOG_WARN(LogCategory::General, QStringLiteral("[Lifecycle] attachMainWindow called more than once! Ignored."));
        Q_ASSERT_X(!m_mainWindowAttached, "ApplicationLifecycleManager", "attachMainWindow must be called exactly once");
        return;
    }

    m_mainWindow = window;
    m_mainWindowAttached = true;
    LOG_INFO(LogCategory::General, QStringLiteral("[Lifecycle] Main window attached to lifecycle manager"));
}

void ApplicationLifecycleManager::requestClose()
{
    if (m_state == ApplicationState::ShuttingDown || m_state == ApplicationState::Terminated)
        return;

    CloseBehavior behavior = closeBehavior();
    LOG_INFO(LogCategory::General,
             QStringLiteral("[Lifecycle] Close requested. Current closeBehavior: %1")
                 .arg(static_cast<int>(behavior)));

    switch (behavior) {
    case CloseBehavior::MinimizeToTray:
        hideApplication();
        break;
    case CloseBehavior::Exit:
        exitApplication();
        break;
    case CloseBehavior::Ask:
    default:
        emit requestCloseConfirmation();
        break;
    }
}

void ApplicationLifecycleManager::setCloseChoice(CloseBehavior choice, bool rememberChoice)
{
    LOG_INFO(LogCategory::General,
             QStringLiteral("[Lifecycle] Close choice set: %1 (remember: %2)")
                 .arg(static_cast<int>(choice))
                 .arg(rememberChoice ? "true" : "false"));

    if (rememberChoice) {
        setCloseBehavior(choice);
    }

    if (choice == CloseBehavior::MinimizeToTray) {
        hideApplication();
    } else {
        exitApplication();
    }
}

void ApplicationLifecycleManager::hideApplication()
{
    if (m_state == ApplicationState::ShuttingDown || m_state == ApplicationState::Terminated)
        return;

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        LOG_WARN(LogCategory::General,
                 QStringLiteral("[Lifecycle] Minimize to tray requested, but system tray is unavailable. Falling back to Exit."));
        exitApplication();
        return;
    }

    setState(ApplicationState::RunningHidden);

    if (m_mainWindow) {
        m_mainWindow->hide();
    }

    LOG_INFO(LogCategory::General, QStringLiteral("[Lifecycle] Application hidden to system tray"));

    // Check if one-time first-hide notification should be shown
    if (m_backend && m_backend->settings()) {
        if (!m_backend->settings()->hasShownFirstHideNotification()) {
            if (m_trayIcon && m_trayIcon->isVisible()) {
                m_trayIcon->showMessage(
                    QStringLiteral("Saiko Soundboard"),
                    QStringLiteral("Saiko Soundboard is running in the background.\nDouble-click the tray icon to reopen."),
                    QSystemTrayIcon::Information,
                    5000
                );
            }
            m_backend->settings()->setHasShownFirstHideNotification(true);
            m_backend->settings()->save();
        }
    }
}

void ApplicationLifecycleManager::restoreWindow()
{
    if (m_state == ApplicationState::ShuttingDown || m_state == ApplicationState::Terminated)
        return;

    setState(ApplicationState::RunningVisible);
    LOG_INFO(LogCategory::General, QStringLiteral("[Lifecycle] Restoring main window"));

    if (m_mainWindow) {
        if (m_mainWindow->windowState() & Qt::WindowMinimized) {
            m_mainWindow->showNormal();
        } else {
            m_mainWindow->show();
        }
        m_mainWindow->raise();
        m_mainWindow->requestActivate();
    }
}

void ApplicationLifecycleManager::exitApplication()
{
    if (m_state == ApplicationState::ShuttingDown || m_state == ApplicationState::Terminated)
        return;

    setState(ApplicationState::ShuttingDown);
    LOG_INFO(LogCategory::General, QStringLiteral("[Lifecycle] Executing canonical shutdown sequence..."));

    if (m_backend) {
        // 1. Save settings
        if (m_backend->settings()) {
            m_backend->settings()->save();
            LOG_INFO(LogCategory::General, QStringLiteral("[Lifecycle] Teardown step 1: Settings saved"));
        }

        // 2. Stop active recordings & audio engine
        if (m_backend->recordingManager()) {
            m_backend->recordingManager()->stopRecording();
            LOG_INFO(LogCategory::General, QStringLiteral("[Lifecycle] Teardown step 2: Recording stopped"));
        }

        // 3. Stop audio playback
        m_backend->stopPlayback();
        LOG_INFO(LogCategory::General, QStringLiteral("[Lifecycle] Teardown step 3: Playback stopped"));

        // 4. Unregister global hotkeys
        if (m_backend->hotkeyManager()) {
            m_backend->hotkeyManager()->unregisterAll();
            LOG_INFO(LogCategory::General, QStringLiteral("[Lifecycle] Teardown step 4: Hotkeys unregistered"));
        }
    }

    // 5. Hide and cleanup system tray icon
    if (m_trayIcon) {
        m_trayIcon->hide();
        LOG_INFO(LogCategory::General, QStringLiteral("[Lifecycle] Teardown step 5: Tray icon hidden"));
    }

    setState(ApplicationState::Terminated);
    LOG_INFO(LogCategory::General, QStringLiteral("[Lifecycle] Shutdown complete. Quitting application loop."));

    QCoreApplication::quit();
}
