#include "FileSink.h"
#include "LogRecord.h"
#include "storage/StoragePaths.h"

#include <QDateTime>
#include <QDir>

namespace Saiko {
namespace Logging {

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

FileSink::FileSink()
{
}

FileSink::~FileSink()
{
    close();
}

// ---------------------------------------------------------------------------
// Open / session header
// ---------------------------------------------------------------------------

bool FileSink::open()
{
    if (m_enabled)
        return true;

    const QString logDir = StoragePaths::logDirectory();
    QDir().mkpath(logDir);

    const QString timestamp = QDateTime::currentDateTime()
                                  .toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
    const QString filePath = logDir + QStringLiteral("/%1.log").arg(timestamp);

    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_enabled = false;
        return false;
    }

    m_stream.setDevice(&m_file);
    m_enabled = true;

    // Record session start and begin monotonic timing
    m_startTime = QDateTime::currentDateTime();
    m_timer.start();

    // Write session header
    const QString now = m_startTime.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    m_stream << QStringLiteral("==================================================\n");
    m_stream << QStringLiteral("Saiko Soundboard Log\n");
    m_stream << QStringLiteral("Session Started: %1\n").arg(now);
    m_stream << QStringLiteral("==================================================\n");

    return true;
}

// ---------------------------------------------------------------------------
// Write
// ---------------------------------------------------------------------------

void FileSink::write(const LogRecord &record)
{
    if (!m_enabled)
        return;

    // 1. Timestamp
    const QString ts = record.timestamp.toString(QStringLiteral("HH:mm:ss.zzz"));

    // 2. Level label (left-aligned, padded)
    const QString level = QString::fromLatin1(logLevelToString(record.level))
                              .leftJustified(6, QLatin1Char(' '), false);

    // 3. Source location: filename:line
    const QString source = QStringLiteral("%1:%2")
                               .arg(QString::fromLatin1(logBasename(record.file)))
                               .arg(record.line);

    // 4. Assemble the formatted line
    //    Format: HH:mm:ss.zzz | LEVEL | Category | file:line | message
    m_stream << QStringLiteral("%1 | %2 | %3 | %4 | %5\n")
                    .arg(ts, -12, QLatin1Char(' '))
                    .arg(level)
                    .arg(QString::fromLatin1(record.category))
                    .arg(source)
                    .arg(record.message);

    // Flush on Error and Critical
    if (record.level >= LogLevel::Error) {
        m_stream.flush();
    }
}

// ---------------------------------------------------------------------------
// Close / session footer
// ---------------------------------------------------------------------------

void FileSink::close()
{
    if (!m_enabled)
        return;

    // Query elapsed before writing footer so the duration includes this operation
    const qint64 elapsedMs = m_timer.elapsed();

    const QString now = QDateTime::currentDateTime()
                            .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));

    // Format elapsed as HH:mm:ss.zzz
    const int hours   = static_cast<int>(elapsedMs / 3600000);
    const int minutes = static_cast<int>((elapsedMs % 3600000) / 60000);
    const int seconds = static_cast<int>((elapsedMs % 60000) / 1000);
    const int millis  = static_cast<int>(elapsedMs % 1000);

    const QString duration = QStringLiteral("%1:%2:%3.%4")
                                 .arg(hours, 2, 10, QLatin1Char('0'))
                                 .arg(minutes, 2, 10, QLatin1Char('0'))
                                 .arg(seconds, 2, 10, QLatin1Char('0'))
                                 .arg(millis, 3, 10, QLatin1Char('0'));

    m_stream << QStringLiteral("==================================================\n");
    m_stream << QStringLiteral("Session End\n");
    m_stream << QStringLiteral("Ended: %1\n").arg(now);
    m_stream << QStringLiteral("Duration: %1\n").arg(duration);
    m_stream << QStringLiteral("==================================================\n");
    m_stream.flush();
    m_file.close();
    m_enabled = false;
}

} // namespace Logging
} // namespace Saiko
