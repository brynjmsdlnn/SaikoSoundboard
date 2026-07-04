#ifndef PLAYERALLOCATOR_H
#define PLAYERALLOCATOR_H

#include <string>

namespace Saiko {
namespace Domain {

class PlayerAllocator {
public:
    std::string allocateId() {
        m_counter++;
        return "player_" + std::to_string(m_counter);
    }

private:
    int m_counter = 0;
};

} // namespace Domain
} // namespace Saiko

#endif // PLAYERALLOCATOR_H
