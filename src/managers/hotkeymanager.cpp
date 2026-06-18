#include "hotkeymanager.h"
#include <QCoreApplication>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

HotkeyManager::HotkeyManager(ActionManager *actionManager, QObject *parent)
    : QObject(parent), m_actionManager(actionManager), m_nextId(1)
{
    qApp->installNativeEventFilter(this);
}

HotkeyManager::~HotkeyManager()
{
    unregisterAll();
    qApp->removeNativeEventFilter(this);
}

bool HotkeyManager::registerHotkey(const QString &keySequence, const Action &action)
{
    if (keySequence.isEmpty()) return false;

    if (m_sequenceToId.contains(keySequence)) {
        emit duplicateDetected(keySequence);
        return false;
    }

    int id = m_nextId++;
    if (nativeRegister(id, keySequence)) {
        m_idToSequence[id] = keySequence;
        m_sequenceToId[keySequence] = id;
        m_idToAction[id] = action;
        return true;
    }

    return false;
}

void HotkeyManager::unregisterAll()
{
    for (int id : m_idToSequence.keys()) {
        nativeUnregister(id);
    }
    m_idToSequence.clear();
    m_sequenceToId.clear();
    m_idToAction.clear();
}

bool HotkeyManager::unregisterHotkey(const QString &keySequence)
{
    if (m_sequenceToId.contains(keySequence)) {
        int id = m_sequenceToId.take(keySequence);
        m_idToSequence.remove(id);
        m_idToAction.remove(id);
        nativeUnregister(id);
        return true;
    }
    return false;
}

bool HotkeyManager::isRegistered(const QString &keySequence) const
{
    return m_sequenceToId.contains(keySequence);
}

void HotkeyManager::updateHotkeys(const QMap<QString, Action> &hotkeyMap)
{
    unregisterAll();
    for (auto it = hotkeyMap.begin(); it != hotkeyMap.end(); ++it) {
        registerHotkey(it.key(), it.value());
    }
}

bool HotkeyManager::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(result);
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG") {
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY) {
            int id = static_cast<int>(msg->wParam);
            if (m_idToAction.contains(id)) {
                if (m_actionManager) {
                    m_actionManager->dispatch(m_idToAction[id]);
                }
                emit hotkeyActivated(m_idToSequence[id]);
                return true;
            }
        }
    }
#endif
    return false;
}

bool HotkeyManager::nativeRegister(int id, const QString &keySequence)
{
#ifdef Q_OS_WIN
    QKeySequence ks(keySequence);
    if (ks.isEmpty()) return false;

    UINT modifiers = getWinModifiers(ks);
    UINT vk = getWinVirtualKey(ks);

    if (vk == 0) return false;

    if (!RegisterHotKey(NULL, id, modifiers, vk)) {
        qWarning() << "HotkeyManager: Failed to register hotkey" << keySequence << "Error:" << GetLastError();
        return false;
    }
    return true;
#else
    Q_UNUSED(id);
    Q_UNUSED(keySequence);
    return false;
#endif
}

void HotkeyManager::nativeUnregister(int id)
{
#ifdef Q_OS_WIN
    UnregisterHotKey(NULL, id);
#else
    Q_UNUSED(id);
#endif
}

#ifdef Q_OS_WIN
UINT HotkeyManager::getWinModifiers(const QKeySequence &ks)
{
    UINT winModifiers = 0;
    // QKeySequence can contain up to 4 keys, but for hotkeys we only use the first one
    int key = ks[0].toCombined();

    if (key & Qt::ControlModifier) winModifiers |= MOD_CONTROL;
    if (key & Qt::AltModifier)     winModifiers |= MOD_ALT;
    if (key & Qt::ShiftModifier)   winModifiers |= MOD_SHIFT;
    if (key & Qt::MetaModifier)    winModifiers |= MOD_WIN;

    return winModifiers;
}

UINT HotkeyManager::getWinVirtualKey(const QKeySequence &ks)
{
    int key = ks[0].toCombined();
    int pureKey = key & ~(Qt::KeyboardModifierMask);

    // Common keys
    if (pureKey >= Qt::Key_A && pureKey <= Qt::Key_Z) return 'A' + (pureKey - Qt::Key_A);
    if (pureKey >= Qt::Key_0 && pureKey <= Qt::Key_9) return '0' + (pureKey - Qt::Key_0);
    if (pureKey >= Qt::Key_F1 && pureKey <= Qt::Key_F24) return VK_F1 + (pureKey - Qt::Key_F1);

    switch (pureKey) {
    case Qt::Key_Space: return VK_SPACE;
    case Qt::Key_Enter:
    case Qt::Key_Return: return VK_RETURN;
    case Qt::Key_Escape: return VK_ESCAPE;
    case Qt::Key_Backspace: return VK_BACK;
    case Qt::Key_Tab: return VK_TAB;
    case Qt::Key_Delete: return VK_DELETE;
    case Qt::Key_Insert: return VK_INSERT;
    case Qt::Key_Home: return VK_HOME;
    case Qt::Key_End: return VK_END;
    case Qt::Key_PageUp: return VK_PRIOR;
    case Qt::Key_PageDown: return VK_NEXT;
    case Qt::Key_Left: return VK_LEFT;
    case Qt::Key_Right: return VK_RIGHT;
    case Qt::Key_Up: return VK_UP;
    case Qt::Key_Down: return VK_DOWN;
    case Qt::Key_Pause: return VK_PAUSE;
    case Qt::Key_CapsLock: return VK_CAPITAL;
    case Qt::Key_Print: return VK_SNAPSHOT;
    default: return 0;
    }
}
#endif
