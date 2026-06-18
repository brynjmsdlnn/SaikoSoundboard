#include "core/adapters/WindowsHotkeyBackend.h"
#include <QDebug>

namespace Saiko {
namespace Adapters {

bool WindowsHotkeyBackend::registerHotkey(int id, const std::string& keySequence) {
    QKeySequence ks(QString::fromStdString(keySequence));
    if (ks.isEmpty()) return false;

    UINT modifiers = getWinModifiers(ks);
    UINT vk = getWinVirtualKey(ks);

    if (vk == 0) return false;

    if (!RegisterHotKey(NULL, id, modifiers, vk)) {
        qWarning() << "WindowsHotkeyBackend: Failed to register hotkey" << QString::fromStdString(keySequence) << "Error:" << GetLastError();
        return false;
    }
    m_registeredIds[id] = true;
    return true;
}

void WindowsHotkeyBackend::unregisterHotkey(int id) {
    if (m_registeredIds.count(id)) {
        UnregisterHotKey(NULL, id);
        m_registeredIds.erase(id);
    }
}

void WindowsHotkeyBackend::unregisterAll() {
    for (auto const& [id, registered] : m_registeredIds) {
        UnregisterHotKey(NULL, id);
    }
    m_registeredIds.clear();
}

UINT WindowsHotkeyBackend::getWinModifiers(const QKeySequence &ks) {
    UINT winModifiers = 0;
    int key = ks[0].toCombined();

    if (key & Qt::ControlModifier) winModifiers |= MOD_CONTROL;
    if (key & Qt::AltModifier)     winModifiers |= MOD_ALT;
    if (key & Qt::ShiftModifier)   winModifiers |= MOD_SHIFT;
    if (key & Qt::MetaModifier)    winModifiers |= MOD_WIN;

    return winModifiers;
}

UINT WindowsHotkeyBackend::getWinVirtualKey(const QKeySequence &ks) {
    int key = ks[0].toCombined();
    int pureKey = key & ~(Qt::KeyboardModifierMask);

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

} // namespace Adapters
} // namespace Saiko
