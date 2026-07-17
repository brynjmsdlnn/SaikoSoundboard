#include "core/adapters/WindowsHotkeyBackend.h"
#include "logging/LogMacros.h"
#include <windows.h>

namespace Saiko {
namespace Adapters {

namespace {

constexpr const char* kNumpadPrefix = "NUM";
constexpr size_t kNumpadPrefixLen = 3;

// Strips a leading "NUM" marker off the final key token (e.g.
// "CTRL+NUM5" -> "CTRL+5", returning true), used to distinguish numpad
// digits from top-row digits. Only matches at the start of the trailing
// token — after the last '+' — so it can't accidentally trigger on
// "NUM" appearing anywhere else in the sequence.
bool stripNumpadPrefix(std::string& seq) {
    size_t tokenStart = seq.find_last_of('+');
    tokenStart = (tokenStart == std::string::npos) ? 0 : tokenStart + 1;

    if (seq.compare(tokenStart, kNumpadPrefixLen, kNumpadPrefix) == 0) {
        seq.erase(tokenStart, kNumpadPrefixLen);
        return true;
    }
    return false;
}

} // namespace

bool WindowsHotkeyBackend::registerHotkey(int id, const std::string& keySequence) {
    std::string cleanSeq = keySequence;
    bool isNumpad = stripNumpadPrefix(cleanSeq);

    QKeySequence ks(QString::fromStdString(cleanSeq));
    if (ks.isEmpty()) return false;

    UINT modifiers = getWinModifiers(ks);
    UINT vk = getWinVirtualKey(ks, isNumpad);
    if (vk == 0) return false;

    if (!RegisterHotKey(NULL, id, modifiers, vk)) {
        LOG_WARN(LogCategory::Hotkeys,
                 QStringLiteral("[HotkeyBackend] Failed to register hotkey (key: \"%1\", error: %2)")
                     .arg(QString::fromStdString(keySequence))
                     .arg(GetLastError()));
        return false;
    }

    m_registeredIds.insert(id);
    return true;
}

void WindowsHotkeyBackend::unregisterHotkey(int id) {
    if (m_registeredIds.count(id)) {
        UnregisterHotKey(NULL, id);
        m_registeredIds.erase(id);
    }
}

void WindowsHotkeyBackend::unregisterAll() {
    for (int id : m_registeredIds) {
        UnregisterHotKey(NULL, id);
    }
    m_registeredIds.clear();
}

UINT WindowsHotkeyBackend::getWinModifiers(const QKeySequence& ks) {
    UINT winModifiers = 0;
    int key = ks[0].toCombined();
    if (key & Qt::ControlModifier) winModifiers |= MOD_CONTROL;
    if (key & Qt::AltModifier)     winModifiers |= MOD_ALT;
    if (key & Qt::ShiftModifier)   winModifiers |= MOD_SHIFT;
    if (key & Qt::MetaModifier)    winModifiers |= MOD_WIN;
    return winModifiers;
}

// Maps a parsed key to its Windows virtual-key code. Intentionally covers
// only what HotkeyCard.qml's capture UI can produce (letters, digits,
// space, enter, tab) — Escape and Backspace are consumed by the UI for
// cancel/clear and never reach here. If the capture UI is extended to
// support more keys (e.g. F-keys, arrows), add them here too.
UINT WindowsHotkeyBackend::getWinVirtualKey(const QKeySequence& ks, bool isNumpad) {
    int key = ks[0].toCombined();
    int pureKey = key & ~(Qt::KeyboardModifierMask);

    if (pureKey >= Qt::Key_A && pureKey <= Qt::Key_Z) return 'A' + (pureKey - Qt::Key_A);

    if (pureKey >= Qt::Key_0 && pureKey <= Qt::Key_9) {
        if (isNumpad) return VK_NUMPAD0 + (pureKey - Qt::Key_0);
        return '0' + (pureKey - Qt::Key_0);
    }

    switch (pureKey) {
    case Qt::Key_Space: return VK_SPACE;
    case Qt::Key_Enter:
    case Qt::Key_Return: return VK_RETURN;
    case Qt::Key_Tab: return VK_TAB;
    default: return 0;
    }
}

} // namespace Adapters
} // namespace Saiko
