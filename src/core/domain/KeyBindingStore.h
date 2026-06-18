#ifndef KEYBINDINGSTORE_H
#define KEYBINDINGSTORE_H

#include <string>
#include <unordered_map>

namespace Saiko {
namespace Domain {

struct Binding {
    std::string keySequence;
    int actionId;
    std::string actionType;
};

class KeyBindingStore {
public:
    void addBinding(const std::string& seq, int actionId, const std::string& type) {
        m_bindings[seq] = {seq, actionId, type};
    }
    
    bool removeBinding(const std::string& seq) {
        return m_bindings.erase(seq) > 0;
    }
    
    bool hasBinding(const std::string& seq) const {
        return m_bindings.find(seq) != m_bindings.end();
    }
    
    void clear() {
        m_bindings.clear();
    }

private:
    std::unordered_map<std::string, Binding> m_bindings;
};

} // namespace Domain
} // namespace Saiko

#endif // KEYBINDINGSTORE_H
