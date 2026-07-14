#include "Logger.h"

#include <QDateTime>

#include <thread>

namespace Saiko {
namespace Logging {

Logger::Logger()
    : QObject(nullptr)
{
}

Logger &Logger::instance()
{
    static Logger s_instance;
    return s_instance;
}

void Logger::log(LogLevel level,
                 const char *category,
                 const char *file,
                 int line,
                 const char *function,
                 const QString &message)
{
    if (!isEnabled(level))
        return;

    // ── Recursion guard ─────────────────────────────────────────────────
    // Prevent infinite re-entrancy if this function is called again while
    // already inside log() — e.g., if a future sink or signal handler
    // triggers another LOG_* call. Re-entrant attempts are silently
    // discarded to avoid stack overflow.
    //
    // Uses an RAII guard so the flag is always restored, even if an
    // exception or early return occurs anywhere below.
    struct LogGuard {
        explicit LogGuard(bool &flag) noexcept : m_flag(flag) { m_flag = true; }
        ~LogGuard() noexcept { m_flag = false; }
        LogGuard(const LogGuard &) = delete;
        LogGuard &operator=(const LogGuard &) = delete;
        bool &m_flag;
    };

    static thread_local bool s_inLog = false;
    if (s_inLog)
        return;
    LogGuard guard(s_inLog);

    LogRecord record;
    record.timestamp = QDateTime::currentDateTime();
    record.level     = level;
    record.category  = category;
    record.file      = file;
    record.line      = line;
    record.function  = function;
    record.message   = message;
    record.threadId  = std::this_thread::get_id();

    m_console.write(record);
    emit logRecordCreated(record);
}

} // namespace Logging
} // namespace Saiko
