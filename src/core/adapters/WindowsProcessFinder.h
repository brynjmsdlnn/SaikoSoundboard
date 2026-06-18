#ifndef WINDOWSPROCESSFINDER_H
#define WINDOWSPROCESSFINDER_H

#include <QString>
#include <QList>
#include <QPair>
#ifdef Q_OS_WIN
#include <windows.h>
#else
typedef unsigned long DWORD;
#endif

namespace Saiko {
namespace Adapters {

class WindowsProcessFinder {
public:
    static DWORD findProcessId(const QString &executableName);
    static QList<QPair<QString, QString>> getRunningProcesses(); // Returns list of (Name, FullPath)
};

} // namespace Adapters
} // namespace Saiko

#endif // WINDOWSPROCESSFINDER_H
