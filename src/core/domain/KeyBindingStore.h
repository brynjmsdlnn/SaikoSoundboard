#ifndef KEYBINDINGSTORE_H
#define KEYBINDINGSTORE_H

#include <string>
#include <unordered_map>
#include <map>

namespace Saiko {
namespace Domain {

class KeyBindingStore {
public:
    int addBinding(const std::string& seq) {
        if (hasBinding(seq)) return -1;
        int id = m_nextId++;
        m_seqToId[seq] = id;
        m_idToSeq[id] = seq;
        return id;
    }
    
    bool removeBindingBySequence(const std::string& seq) {
        if (!hasBinding(seq)) return false;
        int id = m_seqToId[seq];
        m_seqToId.erase(seq);
        m_idToSeq.erase(id);
        return true;
    }

    bool removeBindingById(int id) {
        if (m_idToSeq.find(id) == m_idToSeq.end()) return false;
        std::string seq = m_idToSeq[id];
        m_seqToId.erase(seq);
        m_idToSeq.erase(id);
        return true;
    }
    
    bool hasBinding(const std::string& seq) const {
        return m_seqToId.find(seq) != m_seqToId.end();
    }

    int getId(const std::string& seq) const {
        auto it = m_seqToId.find(seq);
        return it != m_seqToId.end() ? it->second : -1;
    }

    std::string getSequence(int id) const {
        auto it = m_idToSeq.find(id);
        return it != m_idToSeq.end() ? it->second : "";
    }
    
    void clear() {
        m_seqToId.clear();
        m_idToSeq.clear();
        m_nextId = 1;
    }

private:
    std::unordered_map<std::string, int> m_seqToId;
    std::unordered_map<int, std::string> m_idToSeq;
    int m_nextId = 1;
};

} // namespace Domain
} // namespace Saiko

#endif // KEYBINDINGSTORE_H
