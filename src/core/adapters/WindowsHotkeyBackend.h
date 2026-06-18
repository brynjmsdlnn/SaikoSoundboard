#ifndef WINDOWSHOTKEYBACKEND_H
#define WINDOWSHOTKEYBACKEND_H

#include "core/domain/IHotkeyBackend.h"
#include <QKeySequence>
#include <windows.h>
#include <map>

namespace Saiko {
namespace Adapters {

class WindowsHotkeyBackend : public Saiko::Domain::IHotkeyBackend {
public:
    bool registerHotkey(int id, const std::string& keySequence) override;
    void unregisterHotkey(int id) override;
    void unregisterAll() override;

private:
    static UINT getWinModifiers(const QKeySequence &ks);
    static UINT getWinVirtualKey(const QKeySequence &ks);
    std::map<int, bool> m_registeredIds;
};

} // namespace Adapters
} // namespace Saiko

#endif // WINDOWSHOTKEYBACKEND_H
