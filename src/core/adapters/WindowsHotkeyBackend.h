#ifndef WINDOWSHOTKEYBACKEND_H
#define WINDOWSHOTKEYBACKEND_H

#include <QKeySequence>
#include <windows.h>
#include <map>
#include <string>

namespace Saiko {
namespace Adapters {

class WindowsHotkeyBackend {
public:
    bool registerHotkey(int id, const std::string& keySequence);
    void unregisterHotkey(int id);
    void unregisterAll();

private:
    static UINT getWinModifiers(const QKeySequence &ks);
    static UINT getWinVirtualKey(const QKeySequence &ks);
    std::map<int, bool> m_registeredIds;
};

} // namespace Adapters
} // namespace Saiko

#endif // WINDOWSHOTKEYBACKEND_H
