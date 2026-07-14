#ifndef SAIKO_LOGGING_CONSOLESINK_H
#define SAIKO_LOGGING_CONSOLESINK_H

#include "LogLevel.h"

#include <QString>

namespace Saiko {
namespace Logging {

struct LogRecord;

// Formats LogRecord objects and writes them to the Qt debug console.
// Responsible for all formatting: timestamps, alignment, colors, source location.
// Colors are isolated inside this class — no other component knows about ANSI codes.
class ConsoleSink
{
public:
    ConsoleSink();

    // Write a formatted LogRecord to the console.
    // All formatting (timestamp, alignment, colors, source location)
    // happens here and only here.
    void write(const LogRecord &record);

    // Attempt to enable ANSI/VT escape code processing on the current platform.
    // On Windows 10+ this calls SetConsoleMode with ENABLE_VIRTUAL_TERMINAL_PROCESSING.
    // On other platforms ANSI is assumed to be available by default.
    // Safe to call multiple times.
    static void tryEnableColors();

private:
    // Returns the ANSI color code for the given level, or "" if colors are disabled.
    static const char *colorCode(LogLevel level) noexcept;
    static const char *resetCode() noexcept;

    // Whether colors have been successfully enabled.
    // May be set to false on old Windows terminals or non-interactive output.
    static bool s_colorsEnabled;

    // Whether stdout/stderr were successfully redirected to the console device.
    // When false, fputs-based output is skipped and Qt debug channels are used.
    static bool s_consoleReady;
};

} // namespace Logging
} // namespace Saiko

#endif // SAIKO_LOGGING_CONSOLESINK_H
