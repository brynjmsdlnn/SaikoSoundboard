#ifndef IAUDIOOUTPUT_H
#define IAUDIOOUTPUT_H

#include <string>

namespace Saiko {
namespace Domain {

class IAudioOutput {
public:
    virtual ~IAudioOutput() = default;
    virtual void load(const std::string& filePath) = 0;
    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void setVolume(float volume) = 0;
    virtual std::string state() const = 0;
};

} // namespace Domain
} // namespace Saiko

#endif // IAUDIOOUTPUT_H
