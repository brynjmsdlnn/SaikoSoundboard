#ifndef SAIKO_LOGGING_CONSOLESINK_H
#define SAIKO_LOGGING_CONSOLESINK_H

#include "LogLevel.h"

#include <QString>

namespace Saiko {
namespace Logging {

// Formats log messages and writes them to the Qt debug console.
// This is the only built-in sink for Phase 1.
class ConsoleSink
{
public:
    void write(LogLevel level,
               const char *category,
               const char *file,
               int line,
               const char *function,
               const QString &message);
};

} // namespace Logging
} // namespace Saiko

#endif // SAIKO_LOGGING_CONSOLESINK_H
