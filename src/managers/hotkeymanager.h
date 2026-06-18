#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QMap>
#include <QKeySequence>
#include "managers/actionmanager.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class HotkeyManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit HotkeyManager(ActionManager *actionManager, QObject *parent = nullptr);
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
    QMap<int, QString> m_idToSequence;
    QMap<QString, int> m_sequenceToId;
    QMap<int, Action> m_idToAction;
    int m_nextId;

    bool nativeRegister(int id, const QString &keySequence);
    void nativeUnregister(int id);

#ifdef Q_OS_WIN
    static UINT getWinModifiers(const QKeySequence &ks);
    static UINT getWinVirtualKey(const QKeySequence &ks);
#endif
};

#endif // HOTKEYMANAGER_H
