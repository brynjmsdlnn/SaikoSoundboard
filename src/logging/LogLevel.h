#ifndef SAIKO_LOGGING_LOGLEVEL_H
#define SAIKO_LOGGING_LOGLEVEL_H


namespace Saiko {
namespace Logging {

enum class LogLevel
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

// Returns a short uppercase string suitable for log output, e.g. "TRACE", "DEBUG".
// Returns a pointer to a static string literal — safe to convert to QStringView at call site.
inline const char *logLevelToString(LogLevel level) noexcept
{
    switch (level) {
    case LogLevel::Trace:    return "TRACE";
    case LogLevel::Debug:    return "DEBUG";
    case LogLevel::Info:     return "INFO";
    case LogLevel::Warning:  return "WARN";
    case LogLevel::Error:    return "ERROR";
    case LogLevel::Critical: return "CRIT";
    }
    return "?";
}

} // namespace Logging
} // namespace Saiko

#endif // SAIKO_LOGGING_LOGLEVEL_H
