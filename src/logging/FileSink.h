#ifndef SAIKO_LOGGING_FILESINK_H
#define SAIKO_LOGGING_FILESINK_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QString>
#include <QTextStream>

namespace Saiko {
namespace Logging {

struct LogRecord;

// Persists LogRecord entries to a session log file.
// Creates one file per application launch in StoragePaths::logDirectory().
// Manages session timing with a monotonic QElapsedTimer.
// Writes a session header on open and a footer (with duration) on close.
// Flushes immediately on Error and Critical levels.
// Silently disables itself if the log file cannot be created.
class FileSink
{
public:
    FileSink();
    ~FileSink();

    // Open a new session log file and write the session header.
    // Returns true if the file was successfully opened, false otherwise.
    // On failure, all subsequent write() calls are no-ops.
    bool open();

    // Write a formatted LogRecord to the file.
    // No-op if the file is not open.
    void write(const LogRecord &record);

    // Write the session footer (including duration), flush, and close the file.
    // Safe to call multiple times.
    void close();

private:
    QFile m_file;
    QTextStream m_stream;
    QElapsedTimer m_timer;
    QDateTime m_startTime;
    bool m_enabled = false;
};

} // namespace Logging
} // namespace Saiko

#endif // SAIKO_LOGGING_FILESINK_H
