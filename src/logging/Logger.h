#ifndef SAIKO_LOGGING_LOGGER_H
#define SAIKO_LOGGING_LOGGER_H

#include "LogLevel.h"
#include "ConsoleSink.h"

namespace Saiko {
namespace Logging {

// Minimal singleton logger. Owns one ConsoleSink directly —
// no backend abstraction, no dynamic registration in Phase 1.
class Logger
{
public:
    static Logger &instance();

    void log(LogLevel level,
             const char *category,
             const char *file,
             int line,
             const char *function,
             const QString &message);

    void setMinimumLevel(LogLevel level) noexcept { m_level = level; }
    LogLevel minimumLevel() const noexcept { return m_level; }

    // Convenience: can a message at this level be logged?
    bool isEnabled(LogLevel level) const noexcept { return level >= m_level; }

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    LogLevel m_level = LogLevel::Debug;
    ConsoleSink m_console;
};

} // namespace Logging
} // namespace Saiko

#endif // SAIKO_LOGGING_LOGGER_H
