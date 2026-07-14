#ifndef SAIKO_LOGGING_LOGGER_H
#define SAIKO_LOGGING_LOGGER_H

#include <QObject>
#include "LogLevel.h"
#include "LogRecord.h"
#include "ConsoleSink.h"

namespace Saiko {
namespace Logging {

// Singleton logger. Owns one ConsoleSink directly.
//
// Responsibilities:
//   - minimum level filtering
//   - creating LogRecord from raw parameters
//   - dispatching to ConsoleSink
//   - emitting logRecordCreated for in-app log viewers
//
// Formatting is the sole responsibility of ConsoleSink.
// This separation means future sinks (FileSink, spdlog, etc.)
// require no changes to Logger.
class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger &instance();

    // Create a LogRecord and dispatch it to ConsoleSink.
    // Source metadata (file, line, function) is injected by LOG_* macros.
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

signals:
    // Emitted after a LogRecord is created and dispatched to ConsoleSink.
    // Connect LogModel to this signal to receive live log entries.
    void logRecordCreated(const Saiko::Logging::LogRecord &record);

private:
    Logger();
    ~Logger() override = default;
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    LogLevel m_level = LogLevel::Debug;
    ConsoleSink m_console;
};

} // namespace Logging
} // namespace Saiko

#endif // SAIKO_LOGGING_LOGGER_H
