#ifndef SAIKO_LOGGING_LOGGING_H
#define SAIKO_LOGGING_LOGGING_H

#include "LogLevel.h"

namespace Saiko {
namespace Logging {

// Initializes the logging system. Must be called once after QGuiApplication
// construction, before any LOG_*() macro is used.
// Delegates to Logger::initialize() which owns all sink setup.
void initialize(LogLevel minimumLevel = LogLevel::Debug) noexcept;

// Shuts down the logging system. Safe to call multiple times.
// Delegates to Logger::shutdown() which owns all sink teardown.
void shutdown() noexcept;

} // namespace Logging
} // namespace Saiko

#endif // SAIKO_LOGGING_LOGGING_H
