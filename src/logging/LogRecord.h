#ifndef SAIKO_LOGGING_LOGRECORD_H
#define SAIKO_LOGGING_LOGRECORD_H

#include "LogLevel.h"

#include <QDateTime>
#include <QString>
#include <QMetaType>

#include <thread>

namespace Saiko {
namespace Logging {

// Represents a single logging event with all metadata a sink may need.
// Logger creates LogRecord instances and forwards them to sinks.
// Sinks consume LogRecord for formatting and output.
struct LogRecord {
    QDateTime timestamp;
    LogLevel level;
    const char *category;   // Points to a static string literal in LogCategory
    QString message;
    const char *file;       // __FILE__
    int line;               // __LINE__
    const char *function;   // __func__ / __FUNCTION__
    std::thread::id threadId;
};

// ---------------------------------------------------------------------------
// Helper: extract basename from a __FILE__-style path
// ---------------------------------------------------------------------------

// Returns a pointer into the input string, past the last '/' or '\\' separator.
// The returned pointer is guaranteed to be non-null and valid for at least as
// long as the input. If no separator is found, returns the input unchanged.
// This is a minimal scan — no allocation, no copies.
inline const char *logBasename(const char *path) noexcept
{
    if (!path)
        return path;
    const char *slash = path;
    for (const char *p = path; *p; ++p) {
        if (*p == '/' || *p == '\\')
            slash = p + 1;
    }
    return slash;
}

} // namespace Logging
} // namespace Saiko

Q_DECLARE_METATYPE(Saiko::Logging::LogRecord)

#endif // SAIKO_LOGGING_LOGRECORD_H
