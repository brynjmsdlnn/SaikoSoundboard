#include "WindowsProcessFinder.h"
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>

// IAudioMeterInformation is not fully defined in MinGW's endpointvolume.h
struct IAudioMeterInformation : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetPeakValue(float *pfPeak) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMeteringChannelCount(UINT *pnChannelCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetChannelsPeakValues(UINT32 u32ChannelCount, float *afPeakValues) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryHardwareSupport(DWORD *pdwHardwareSupportMask) = 0;
};
const IID IID_IAudioMeterInformation_Local = { 0xC02216F6, 0x8C67, 0x4B5B, { 0x9D, 0x00, 0xD0, 0x08, 0xE7, 0x3E, 0x00, 0x64 } };
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

QSet<QString> WindowsProcessFinder::getProcessesProducingSound() {
    QSet<QString> result;
#ifdef Q_OS_WIN
    bool needsUninit = false;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr)) {
        needsUninit = true;
    } else if (hr == RPC_E_CHANGED_MODE) {
        // Already initialized
    } else {
        return result;
    }
    
    IMMDeviceEnumerator* pEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (SUCCEEDED(hr)) {
        IMMDevice* pDevice = NULL;
        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
        if (SUCCEEDED(hr)) {
            IAudioSessionManager2* pSessionManager = NULL;
            hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&pSessionManager);
            if (SUCCEEDED(hr)) {
                IAudioSessionEnumerator* pSessionEnumerator = NULL;
                hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
                if (SUCCEEDED(hr)) {
                    int sessionCount = 0;
                    pSessionEnumerator->GetCount(&sessionCount);
                    for (int i = 0; i < sessionCount; i++) {
                        IAudioSessionControl* pSessionControl = NULL;
                        if (SUCCEEDED(pSessionEnumerator->GetSession(i, &pSessionControl))) {
                            IAudioSessionControl2* pSessionControl2 = NULL;
                            if (SUCCEEDED(pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2))) {
                                DWORD sessionPid = 0;
                                if (SUCCEEDED(pSessionControl2->GetProcessId(&sessionPid)) && sessionPid != 0) {
                                    bool isSystemSounds = (pSessionControl2->IsSystemSoundsSession() == S_OK);
                                    if (!isSystemSounds) {
                                        IAudioMeterInformation* pMeterInfo = NULL;
                                        if (SUCCEEDED(pSessionControl->QueryInterface(IID_IAudioMeterInformation_Local, (void**)&pMeterInfo))) {
                                            float peakValue = 0.0f;
                                            if (SUCCEEDED(pMeterInfo->GetPeakValue(&peakValue)) && peakValue > 0.0f) {
                                                HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, sessionPid);
                                                if (hProcess) {
                                                    WCHAR szPath[MAX_PATH];
                                                    if (GetModuleFileNameExW(hProcess, NULL, szPath, MAX_PATH)) {
                                                        QString exeName = QFileInfo(QString::fromWCharArray(szPath)).fileName();
                                                        if (!exeName.isEmpty()) {
                                                            result.insert(exeName.toLower());
                                                        }
                                                    }
                                                    CloseHandle(hProcess);
                                                }
                                            }
                                            pMeterInfo->Release();
                                        }
                                    }
                                }
                                pSessionControl2->Release();
                            }
                            pSessionControl->Release();
                        }
                    }
                    pSessionEnumerator->Release();
                }
                pSessionManager->Release();
            }
            pDevice->Release();
        }
        pEnumerator->Release();
    }
    
    if (needsUninit) {
        CoUninitialize();
    }
#endif
    return result;
}

} // namespace Adapters
} // namespace Saiko
