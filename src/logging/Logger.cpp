#include "Logger.h"

namespace Saiko {
namespace Logging {

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

    m_console.write(level, category, file, line, function, message);
}

} // namespace Logging
} // namespace Saiko
