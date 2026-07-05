#ifndef WINDOWSHOTKEYBACKEND_H
#define WINDOWSHOTKEYBACKEND_H
#include <QKeySequence>
#include <set>
#include <string>

namespace Saiko {
namespace Adapters {

class WindowsHotkeyBackend {
public:
    bool registerHotkey(int id, const std::string& keySequence);
    void unregisterHotkey(int id);
    void unregisterAll();

private:
    // Plain `unsigned int` (not the WinAPI `UINT` typedef) so this header
    // doesn't need to include <windows.h> — that keeps Windows macros
    // (min/max, near/far, ...) from leaking into every file that includes
    // this one. <windows.h> is only pulled in by the .cpp, where the actual
    // WinAPI calls live.
    static unsigned int getWinModifiers(const QKeySequence &ks);
    static unsigned int getWinVirtualKey(const QKeySequence &ks, bool isNumpad);

    std::set<int> m_registeredIds;
};

} // namespace Adapters
} // namespace Saiko
#endif // WINDOWSHOTKEYBACKEND_H