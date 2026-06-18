#ifndef IHOTKEYBACKEND_H
#define IHOTKEYBACKEND_H

#include <string>

namespace Saiko {
namespace Domain {

class IHotkeyBackend {
public:
    virtual ~IHotkeyBackend() = default;
    virtual bool registerHotkey(int id, const std::string& keySequence) = 0;
    virtual void unregisterHotkey(int id) = 0;
    virtual void unregisterAll() = 0;
};

} // namespace Domain
} // namespace Saiko

#endif // IHOTKEYBACKEND_H
