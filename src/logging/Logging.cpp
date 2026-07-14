#include "Logging.h"
#include "LogRecord.h"
#include "Logger.h"

#include <QMetaType>

namespace Saiko {
namespace Logging {

void initialize(LogLevel minimumLevel) noexcept
{
    // Register LogRecord metatype for queued signal-slot connections
    // (required when Logger::log() is called from background threads).
    qRegisterMetaType<LogRecord>("Saiko::Logging::LogRecord");

    Logger::instance().initialize(minimumLevel);
}

void shutdown() noexcept
{
    Logger::instance().shutdown();
}

} // namespace Logging
} // namespace Saiko
