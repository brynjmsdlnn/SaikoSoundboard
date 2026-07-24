#ifndef APPLICATIONLIFECYCLEMANAGER_H
#define APPLICATIONLIFECYCLEMANAGER_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QPointer>
#include <QColor>
#include "managers/settingsmanager.h"

class QQuickWindow;
class QmlBackend;

class ApplicationLifecycleManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SettingsManager::CloseBehavior closeBehavior READ closeBehavior WRITE setCloseBehavior NOTIFY closeBehaviorChanged)
    Q_PROPERTY(ApplicationState state READ state NOTIFY stateChanged)

public:
    using CloseBehavior = SettingsManager::CloseBehavior;

    enum class ApplicationState {
        RunningVisible = 0,
        RunningHidden = 1,
        ShuttingDown = 2,
        Terminated = 3
    };
    Q_ENUM(ApplicationState)

    explicit ApplicationLifecycleManager(QmlBackend *backend, QObject *parent = nullptr);
    ~ApplicationLifecycleManager() override;

    CloseBehavior closeBehavior() const;
    void setCloseBehavior(CloseBehavior behavior);

    ApplicationState state() const { return m_state; }

    Q_INVOKABLE void attachMainWindow(QQuickWindow *window);
    Q_INVOKABLE void requestClose();
    Q_INVOKABLE void setCloseChoice(CloseBehavior choice, bool rememberChoice);
    Q_INVOKABLE void hideApplication();
    Q_INVOKABLE void restoreWindow();
    Q_INVOKABLE void exitApplication();

signals:
    void closeBehaviorChanged();
    void stateChanged();
    void requestCloseConfirmation();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onTrayIconColorChanged();

private:
    void setupTrayIcon();
    void updateTrayIcon();
    QIcon createColoredIcon(const QColor &color) const;
    void setState(ApplicationState newState);

    QmlBackend *m_backend = nullptr;
    QPointer<QQuickWindow> m_mainWindow;
    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_trayMenu = nullptr;
    QAction *m_openAction = nullptr;
    QAction *m_exitAction = nullptr;

    ApplicationState m_state = ApplicationState::RunningVisible;
    bool m_mainWindowAttached = false;
};

#endif // APPLICATIONLIFECYCLEMANAGER_H
