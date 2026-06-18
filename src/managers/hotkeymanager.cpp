#include "hotkeymanager.h"
#include <QCoreApplication>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

HotkeyManager::HotkeyManager(ActionManager *actionManager, Saiko::Adapters::WindowsHotkeyBackend *backend, QObject *parent)
    : QObject(parent), m_actionManager(actionManager), m_backend(backend)
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

    if (m_store.hasBinding(keySequence.toStdString())) {
        emit duplicateDetected(keySequence);
        return false;
    }

    int id = m_store.addBinding(keySequence.toStdString());
    if (id == -1) return false;

    if (m_backend->registerHotkey(id, keySequence.toStdString())) {
        m_idToAction[id] = action;
        return true;
    } else {
        m_store.removeBindingById(id);
    }

    return false;
}

void HotkeyManager::unregisterAll()
{
    if (m_backend) {
        m_backend->unregisterAll();
    }
    m_store.clear();
    m_idToAction.clear();
}

bool HotkeyManager::unregisterHotkey(const QString &keySequence)
{
    int id = m_store.getId(keySequence.toStdString());
    if (id != -1) {
        m_store.removeBindingById(id);
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
    return m_store.hasBinding(keySequence.toStdString());
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
                QString seq = QString::fromStdString(m_store.getSequence(id));
                emit hotkeyActivated(seq);
                return true;
            }
        }
    }
#endif
    return false;
}
