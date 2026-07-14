#include "ConsoleSink.h"

#include <QDebug>
#include <QString>

namespace Saiko {
namespace Logging {

static constexpr int kLevelWidth = 6;  // padded width for level label

void ConsoleSink::write(LogLevel level,
                        const char *category,
                        const char *file,
                        int line,
                        const char *function,
                        const QString &message)
{
    Q_UNUSED(file)
    Q_UNUSED(line)
    Q_UNUSED(function)

    const QString prefix = QStringLiteral("[%1][%2] ")
                               .arg(QString::fromLatin1(logLevelToString(level)), -kLevelWidth, QLatin1Char(' '))
                               .arg(QString::fromLatin1(category));

    switch (level) {
    case LogLevel::Trace:
    case LogLevel::Debug:
        qDebug().noquote() << prefix + message;
        break;
    case LogLevel::Info:
        qInfo().noquote()  << prefix + message;
        break;
    case LogLevel::Warning:
        qWarning().noquote() << prefix + message;
        break;
    case LogLevel::Error:
    case LogLevel::Critical:
        qCritical().noquote() << prefix + message;
        break;
    }
}

} // namespace Logging
} // namespace Saiko
