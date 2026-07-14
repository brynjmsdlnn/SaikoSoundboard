#ifndef SAIKO_LOGGING_LOGGER_H
#define SAIKO_LOGGING_LOGGER_H

#include <QObject>
#include "LogLevel.h"
#include "LogRecord.h"
#include "ConsoleSink.h"
#include "FileSink.h"

namespace Saiko {
namespace Logging {

// Singleton logger. Routes each LogRecord to ConsoleSink, LogModel,
// and FileSink.
//
// Responsibilities:
//   - minimum level filtering
//   - creating LogRecord from raw parameters
//   - dispatching to ConsoleSink (console output)
//   - dispatching to FileSink (session log file)
//   - emitting logRecordCreated for in-app log viewers
//
// Formatting is the responsibility of each sink.
// This separation means future sinks require no changes to Logger.
class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger &instance();

    // Create a LogRecord and dispatch it to all sinks.
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

    // Initialize the logging system: set minimum level, prepare ConsoleSink,
    // and open the session log file via FileSink.
    // Safe to call multiple times.
    void initialize(LogLevel level);

    // Shut down all sinks: write session footer, flush, and close the log file.
    // Safe to call multiple times.
    void shutdown();

signals:
    // Emitted after a LogRecord is created and dispatched to sinks.
    // Connect LogModel to this signal to receive live log entries.
    void logRecordCreated(const Saiko::Logging::LogRecord &record);

private:
    Logger();
    ~Logger() override = default;
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    LogLevel m_level = LogLevel::Debug;
    ConsoleSink m_console;
    FileSink m_file;
};

} // namespace Logging
} // namespace Saiko

#endif // SAIKO_LOGGING_LOGGER_H
