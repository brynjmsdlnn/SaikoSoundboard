#include "ConsoleSink.h"
#include "LogRecord.h"

#include <QDebug>
#include <QString>

#include <cstdio>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Saiko {
namespace Logging {

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

static constexpr int kLevelWidth  = 6;   // padded width for level label
static constexpr int kTimeWidth   = 12;  // HH:mm:ss.zzz

// ---------------------------------------------------------------------------
// Color support
// ---------------------------------------------------------------------------

bool ConsoleSink::s_colorsEnabled = false;
bool ConsoleSink::s_consoleReady = false;

void ConsoleSink::tryEnableColors()
{
#ifdef Q_OS_WIN
    // GUI apps (compiled with -mwindows) have no console by default.
    // AttachConsole inherits the parent process's console (cmd/PowerShell/Windows Terminal).
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        const DWORD err = GetLastError();
        // ERROR_ACCESS_DENIED means we already have a console — continue.
        // ERROR_INVALID_HANDLE means no parent console — colors won't work.
        if (err == ERROR_INVALID_HANDLE) {
            s_colorsEnabled = false;
            s_consoleReady = false;
            return;
        }
    }

    // Redirect stdout/stderr to the console device so fputs/fprintf work after AttachConsole.
    // Check return values: if freopen fails, the stream is closed and left in an
    // indeterminate state. We must not call fputs on it afterwards.
    // When the redirect fails, we gracefully fall through to Qt's debug output below.
    bool okOut = (std::freopen("CONOUT$", "w", stdout) != nullptr);
    bool okErr = (std::freopen("CONOUT$", "w", stderr) != nullptr);

    if (okOut)
        std::setbuf(stdout, nullptr);
    if (okErr)
        std::setbuf(stderr, nullptr);

    // s_consoleReady is true only when BOTH streams were successfully redirected.
    s_consoleReady = okOut && okErr;

    // Enable virtual terminal (ANSI escape) processing on Windows 10+.
    if (s_consoleReady) {
        const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        const HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);

        auto tryEnable = [](HANDLE h) -> bool {
            if (h == INVALID_HANDLE_VALUE || h == nullptr)
                return false;
            DWORD mode = 0;
            if (!GetConsoleMode(h, &mode))
                return false;
            return SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
        };

        s_colorsEnabled = tryEnable(hOut) || tryEnable(hErr);
    }
#else
    // ANSI escape codes are natively supported on Linux / macOS terminals.
    s_consoleReady = true;
    s_colorsEnabled = true;
#endif
}

const char *ConsoleSink::colorCode(LogLevel level) noexcept
{
    if (!s_colorsEnabled)
        return "";

    switch (level) {
    case LogLevel::Trace:    return "\033[90m";   // gray
    case LogLevel::Debug:    return "\033[36m";   // cyan
    case LogLevel::Info:     return "\033[32m";   // green
    case LogLevel::Warning:  return "\033[33m";   // yellow
    case LogLevel::Error:    return "\033[31m";   // red
    case LogLevel::Critical: return "\033[91m";   // bright red
    }
    return "";
}

const char *ConsoleSink::resetCode() noexcept
{
    return s_colorsEnabled ? "\033[0m" : "";
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

ConsoleSink::ConsoleSink()
{
    // Colors are enabled once via Logging::initialize() -> tryEnableColors()
}

// ---------------------------------------------------------------------------
// Formatting & output
// ---------------------------------------------------------------------------

void ConsoleSink::write(const LogRecord &record)
{
    // 1. Timestamp
    const QString ts = record.timestamp.toString(QStringLiteral("HH:mm:ss.zzz"));

    // 2. Level label (left-aligned, padded)
    const QString level = QString::fromLatin1(logLevelToString(record.level))
                              .leftJustified(kLevelWidth, QLatin1Char(' '), false);

    // 3. Source location: filename:line
    //    Use the shared logBasename() helper to strip directory prefixes.
    const QString source = QStringLiteral("%1:%2")
                               .arg(QString::fromLatin1(logBasename(record.file)))
                               .arg(record.line);

    // 4. Assemble the formatted line
    //    Format: HH:mm:ss.zzz | LEVEL | Category | file:line | message
    const QString body = QStringLiteral("%1 | %2 | %3 | %4 | %5")
                             .arg(ts, -kTimeWidth, QLatin1Char(' '))
                             .arg(level)
                             .arg(QString::fromLatin1(record.category))
                             .arg(source)
                             .arg(record.message);

    // Determine which stream to use: stdout for info and below, stderr for warnings+
    const bool isError = (record.level >= LogLevel::Warning);

    if (s_consoleReady) {
        // Direct console output via fputs. If ANSI colors are enabled,
        // colorCode/resetCode inject the VT escape sequences; otherwise
        // they return empty strings and output is plain text.
        //
        // s_consoleReady is false only when no console could be attached
        // or redirected — in that case we fall through to Qt debug channels
        // which are visible in IDEs and debug output viewers.
        const QByteArray line = QStringLiteral("%1%2%3\n")
                                    .arg(QString::fromLatin1(colorCode(record.level)))
                                    .arg(body)
                                    .arg(QString::fromLatin1(resetCode()))
                                    .toUtf8();

        std::fputs(line.constData(), isError ? stderr : stdout);
        return;
    }

    // Fallback: output via Qt's debug channels when no usable console exists.
    // This is visible in IDE debug output (Qt Creator, Visual Studio, etc.).
    switch (record.level) {
    case LogLevel::Trace:
    case LogLevel::Debug:
        qDebug().noquote() << body;
        break;
    case LogLevel::Info:
        qInfo().noquote() << body;
        break;
    case LogLevel::Warning:
        qWarning().noquote() << body;
        break;
    case LogLevel::Error:
    case LogLevel::Critical:
        qCritical().noquote() << body;
        break;
    }
}

} // namespace Logging
} // namespace Saiko
