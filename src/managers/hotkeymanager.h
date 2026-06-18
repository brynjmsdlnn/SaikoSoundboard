#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QMap>
#include "core/adapters/WindowsHotkeyBackend.h"
#include "core/domain/KeyBindingStore.h"
#include "managers/actionmanager.h"

class HotkeyManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit HotkeyManager(ActionManager *actionManager, Saiko::Adapters::WindowsHotkeyBackend *backend, QObject *parent = nullptr);
    ~HotkeyManager();

    // Register a hotkey for a specific action
    bool registerHotkey(const QString &keySequence, const Action &action);
    
    // Unregister all hotkeys or a specific one
    void unregisterAll();
    bool unregisterHotkey(const QString &keySequence);

    // Check if a hotkey is already registered
    bool isRegistered(const QString &keySequence) const;

    // Helper to clear and re-register everything (useful for runtime updates)
    void updateHotkeys(const QMap<QString, Action> &hotkeyMap);

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
