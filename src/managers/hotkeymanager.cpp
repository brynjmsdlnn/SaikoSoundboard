#include "hotkeymanager.h"
#include <QCoreApplication>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

HotkeyManager::HotkeyManager(ActionManager *actionManager, Saiko::Domain::IHotkeyBackend *backend, QObject *parent)
    : QObject(parent), m_actionManager(actionManager), m_backend(backend), m_nextId(1)
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
    if (!m_backend) return false;

    if (m_sequenceToId.contains(keySequence)) {
        emit duplicateDetected(keySequence);
        return false;
    }

    int id = m_nextId++;
    if (m_backend->registerHotkey(id, keySequence.toStdString())) {
        m_idToSequence[id] = keySequence;
        m_sequenceToId[keySequence] = id;
        m_idToAction[id] = action;
        return true;
    }

    return false;
}

void HotkeyManager::unregisterAll()
{
    if (m_backend) {
        m_backend->unregisterAll();
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
        if (m_backend) {
            m_backend->unregisterHotkey(id);
        }
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
