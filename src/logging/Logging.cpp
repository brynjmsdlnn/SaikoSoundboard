#include "Logging.h"
#include "Logger.h"

namespace Saiko {
namespace Logging {

void initialize(LogLevel minimumLevel) noexcept
{
    Logger::instance().setMinimumLevel(minimumLevel);
}

void shutdown() noexcept
{
    // Phase 1: no resources to release.
    // Phase 3+ (FileSink): flush and close files here.
    // Phase 6+ (spdlog): call spdlog::shutdown() here.
}

} // namespace Logging
} // namespace Saiko
