#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <vector>
#include <cstdint>
#include <cstddef>

namespace Saiko {
namespace Domain {

class RingBuffer {
public:
    RingBuffer() : m_maxBytes(0) {}
    
    void setMaxBytes(size_t maxBytes) {
        m_maxBytes = maxBytes;
        if (m_buffer.size() > m_maxBytes) {
            m_buffer.erase(m_buffer.begin(), m_buffer.begin() + (m_buffer.size() - m_maxBytes));
        }
    }
    
    size_t maxBytes() const { return m_maxBytes; }
    
    void push(const uint8_t* data, size_t size) {
        if (m_maxBytes == 0 || size == 0) return;
        m_buffer.insert(m_buffer.end(), data, data + size);
        if (m_buffer.size() > m_maxBytes) {
            size_t overflow = m_buffer.size() - m_maxBytes;
            m_buffer.erase(m_buffer.begin(), m_buffer.begin() + overflow);
        }
    }
    
    const std::vector<uint8_t>& data() const { return m_buffer; }
    
    void clear() {
        m_buffer.clear();
    }
    
    size_t size() const { return m_buffer.size(); }

private:
    std::vector<uint8_t> m_buffer;
    size_t m_maxBytes;
};

} // namespace Domain
} // namespace Saiko

#endif // RINGBUFFER_H
