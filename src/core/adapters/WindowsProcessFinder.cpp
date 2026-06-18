#include "WindowsProcessFinder.h"
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

namespace Saiko {
namespace Adapters {

DWORD WindowsProcessFinder::findProcessId(const QString &executableName) {
    DWORD pid = 0;
#ifdef Q_OS_WIN
    DWORD processes[1024], cbNeeded, cProcesses;
    if (EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        cProcesses = cbNeeded / sizeof(DWORD);
        for (unsigned int i = 0; i < cProcesses; i++) {
            if (processes[i] != 0) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
                if (hProcess) {
                    WCHAR szPath[MAX_PATH];
                    if (GetModuleFileNameExW(hProcess, NULL, szPath, MAX_PATH)) {
                        QString exeName = QFileInfo(QString::fromWCharArray(szPath)).fileName();
                        if (exeName.compare(executableName, Qt::CaseInsensitive) == 0) {
                            pid = processes[i];
                            CloseHandle(hProcess);
                            break;
                        }
                    }
                    CloseHandle(hProcess);
                }
            }
        }
    }
#endif
    return pid;
}

QList<QPair<QString, QString>> WindowsProcessFinder::getRunningProcesses() {
    QList<QPair<QString, QString>> result;
#ifdef Q_OS_WIN
    DWORD processes[1024], cbNeeded, cProcesses;
    if (EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        cProcesses = cbNeeded / sizeof(DWORD);
        for (unsigned int i = 0; i < cProcesses; i++) {
            if (processes[i] != 0) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
                if (hProcess) {
                    WCHAR szPath[MAX_PATH];
                    if (GetModuleFileNameExW(hProcess, NULL, szPath, MAX_PATH)) {
                        QString fullPath = QString::fromWCharArray(szPath);
                        QFileInfo fileInfo(fullPath);
                        QString name = fileInfo.fileName();
                        
                        if (!name.isEmpty()) {
                            result.append(qMakePair(name, fullPath));
                        }
                    }
                    CloseHandle(hProcess);
                }
            }
        }
    }
#endif
    return result;
}

} // namespace Adapters
} // namespace Saiko
