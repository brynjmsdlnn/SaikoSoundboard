#ifndef SAIKO_LOGGING_LOGGING_H
#define SAIKO_LOGGING_LOGGING_H

#include "LogLevel.h"

namespace Saiko {
namespace Logging {

// Initializes the logging system. Must be called once after QGuiApplication
// construction, before any LOG_*() macro is used.
void initialize(LogLevel minimumLevel = LogLevel::Debug) noexcept;

// Shuts down the logging system. Safe to call multiple times.
// Currently a no-op placeholder: the logging subsystem has no resources
// that require explicit teardown. Future phases (FileSink, spdlog) will
// add flush-and-close logic here.
void shutdown() noexcept;

} // namespace Logging
} // namespace Saiko

#endif // SAIKO_LOGGING_LOGGING_H
