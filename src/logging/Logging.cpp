#include "Logging.h"
#include "ConsoleSink.h"
#include "Logger.h"
#include "LogRecord.h"

#include <QMetaType>

namespace Saiko {
namespace Logging {

void initialize(LogLevel minimumLevel) noexcept
{
    // Register LogRecord metatype for queued signal-slot connections
    // (required when Logger::log() is called from background threads).
    qRegisterMetaType<LogRecord>("Saiko::Logging::LogRecord");

    ConsoleSink::tryEnableColors();
    Logger::instance().setMinimumLevel(minimumLevel);
}

void shutdown() noexcept
{
    // No-op: the logging subsystem has no resources that require
    // explicit teardown in the current phase. This function exists
    // so future phases (FileSink flush/close, spdlog shutdown) have
    // a defined call site without changing the public API.
}

} // namespace Logging
} // namespace Saiko
