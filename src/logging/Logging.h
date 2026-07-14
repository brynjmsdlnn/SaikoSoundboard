#ifndef SAIKO_LOGGING_LOGGING_H
#define SAIKO_LOGGING_LOGGING_H

#include "LogLevel.h"

namespace Saiko {
namespace Logging {

// Initializes the logging system. Must be called once after QGuiApplication
// construction, before any LOG_*() macro is used.
void initialize(LogLevel minimumLevel = LogLevel::Debug) noexcept;

// Shuts down the logging system. Flushes and releases any resources held by
// registered sinks. Safe to call multiple times. After shutdown, LOG_*()
// macros become no-ops.
void shutdown() noexcept;

} // namespace Logging
} // namespace Saiko

#endif // SAIKO_LOGGING_LOGGING_H
