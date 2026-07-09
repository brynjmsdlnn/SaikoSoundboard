#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QMap>
#include "core/domain/KeyBindingStore.h"
#include "managers/actionmanager.h"

namespace Saiko {
namespace Adapters {
class WindowsHotkeyBackend;
}
}

class HotkeyManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit HotkeyManager(ActionManager *actionManager, Saiko::Adapters::WindowsHotkeyBackend *backend, QObject *parent = nullptr);
    ~HotkeyManager();

    // Register a hotkey for a specific action
    Q_INVOKABLE bool registerHotkey(const QString &keySequence, const Action &action);

    // Unregister all hotkeys or a specific one
    Q_INVOKABLE void unregisterAll();
    Q_INVOKABLE bool unregisterHotkey(const QString &keySequence);

    // Check if a hotkey is already registered
    Q_INVOKABLE bool isRegistered(const QString &keySequence) const;

    // Helper to clear and re-register everything (useful for runtime updates)
    Q_INVOKABLE void updateHotkeys(const QMap<QString, Action> &hotkeyMap);

    // From QAbstractNativeEventFilter
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

signals:
    void hotkeyActivated(const QString &keySequence);
    void duplicateDetected(const QString &keySequence);

private:
    ActionManager *m_actionManager;
    Saiko::Adapters::WindowsHotkeyBackend *m_backend;
    Saiko::Domain::KeyBindingStore m_store;
    QMap<int, Action> m_idToAction;
};

#endif // HOTKEYMANAGER_H
