#ifndef SAIKO_LOGGING_LOGMACROS_H
#define SAIKO_LOGGING_LOGMACROS_H

// ---------------------------------------------------------------------------
// Public logging API — the ONLY thing most files should include.
//
// Usage:
//   LOG_INFO(LogCategory::Playback, QStringLiteral("Playing %1").arg(file));
//   LOG_WARN(LogCategory::Audio, "Device initialization failed");
//
// These macros always capture source location (file, line, function)
// even if the sink ignores them today. This future-proofs the API.
// ---------------------------------------------------------------------------

#include "Logger.h"
#include "LogCategory.h"

// ---------------------------------------------------------------------------
// Internal: level check + dispatch
// ---------------------------------------------------------------------------

#define SAIKO_LOG_INTERNAL(level_, category_, message_)                        \
    do {                                                                       \
        auto &_slog = ::Saiko::Logging::Logger::instance();                    \
        if (_slog.isEnabled(level_)) {                                         \
            _slog.log((level_), (category_), __FILE__, __LINE__, __FUNCTION__, \
                      (message_));                                             \
        }                                                                      \
    } while (false)

// ---------------------------------------------------------------------------
// Public macros
// ---------------------------------------------------------------------------

#define LOG_TRACE(category, message) \
    SAIKO_LOG_INTERNAL(::Saiko::Logging::LogLevel::Trace,    (category), (message))

#define LOG_DEBUG(category, message) \
    SAIKO_LOG_INTERNAL(::Saiko::Logging::LogLevel::Debug,    (category), (message))

#define LOG_INFO(category, message) \
    SAIKO_LOG_INTERNAL(::Saiko::Logging::LogLevel::Info,     (category), (message))

#define LOG_WARN(category, message) \
    SAIKO_LOG_INTERNAL(::Saiko::Logging::LogLevel::Warning,  (category), (message))

#define LOG_ERROR(category, message) \
    SAIKO_LOG_INTERNAL(::Saiko::Logging::LogLevel::Error,    (category), (message))

#define LOG_CRITICAL(category, message) \
    SAIKO_LOG_INTERNAL(::Saiko::Logging::LogLevel::Critical, (category), (message))

#endif // SAIKO_LOGGING_LOGMACROS_H
