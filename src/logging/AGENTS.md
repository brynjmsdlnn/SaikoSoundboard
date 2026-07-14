# Logging Subsystem

## Purpose

Lightweight logging foundation for Saiko Soundboard. Routes structured log records through multiple sinks with a single-point façade.

## Ownership

- `Logger.h/.cpp` — Singleton. Owns sink lifecycle (`initialize`/`shutdown`) and dispatches `LogRecord` to ConsoleSink, FileSink, and the `logRecordCreated` signal (consumed by LogModel).
- `ConsoleSink.h/.cpp` — Formats `LogRecord` with ANSI colors and writes to the debug console (stdout/stderr).
- `FileSink.h/.cpp` — Persists `LogRecord` to a timestamped session file (`YYYY-MM-DD_HH-MM-SS.log`) in `StoragePaths::logDirectory()`. Manages monotonic session timing via `QElapsedTimer`.
- `LogModel.h/.cpp` — `QAbstractListModel` that buffers recent log entries and exposes them to QML via `LogViewer`.
- `LogRecord.h` — Value type with timestamp, level, category, source location, message, thread ID. Inline helpers: `logBasename()`.
- `LogLevel.h` — `LogLevel` enum (Trace, Debug, Info, Warning, Error, Critical) + `logLevelToString()`.
- `LogCategory.h` — Static string constants for categorization (Audio, Playback, Recording, etc.).
- `LogMacros.h` — Public `LOG_*` macros (LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_CRITICAL).
- `Logging.h/.cpp` — Public façade: `Logging::initialize(level)` and `Logging::shutdown()`. Both delegate to `Logger`.

## Local Contracts

- **Logger** is a singleton — use `Logger::instance()` everywhere.
- **Only `Logging` namespace functions** are the public API called from application code (`main.cpp` calls `Logging::initialize()`).
- **Logger owns sink routing** — `Logging.cpp` must not call sink methods directly.
- **Routing order** inside `Logger::log()`:
  1. `m_console.write(record)`
  2. `m_file.write(record)`
  3. `emit logRecordCreated(record)` (→ LogModel)
- **Lifecycle**:
  - `Logging::initialize(level)` → `Logger::instance().initialize(level)` → `ConsoleSink::tryEnableColors()`, `setMinimumLevel(level)`, `FileSink::open()` (writes session header, starts QElapsedTimer)
  - `Logging::shutdown()` → `Logger::instance().shutdown()` → `FileSink::close()` (writes session footer with Ended + Duration, flushes, closes)
- **Recursion guard**: `Logger::log()` uses a `thread_local` RAII guard. Re-entrant calls are silently discarded.
- **Thread safety**: No mutexes, no async queues, no worker threads. Same threading model as the rest of the application.
- **FileSink flush policy**: Flush only on Error and Critical. No timer-based flushing.
- **FileSink failure**: Silently disables itself if the log file cannot be created. Never crashes the application.
- **LogModel** connects to `Logger::logRecordCreated` signal for in-app log viewing.
- **No sink hierarchy or interfaces** — sinks are concrete classes owned directly by Logger.

## Verification

- Build succeeds: `cmake --build build --target SaikoLogging`
- Log file appears in `StoragePaths::logDirectory()` after `Logging::initialize()`
- Session header/footer with duration are written correctly
- Console output unchanged
- LogModel in QML receives records as before

## Child DOX Index

No children — all files organized under this directory.
