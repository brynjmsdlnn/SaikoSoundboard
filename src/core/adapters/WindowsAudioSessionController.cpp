#include "WindowsAudioSessionController.h"
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <psapi.h>
#include <QFileInfo>
#include <QDebug>

namespace Saiko {
namespace Adapters {

bool WindowsAudioSessionController::setAbsoluteMuteExcept(const QSet<QString>& exemptExes, bool muteActive)
{
    HRESULT hr;
    
    IMMDeviceEnumerator* pEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        qDebug() << "WASAPI: CoCreateInstance MMDeviceEnumerator failed. hr = 0x" << QString::number(hr, 16);
        return false;
    }
    
    IMMDevice* pDevice = NULL;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    pEnumerator->Release();
    if (FAILED(hr)) {
        qDebug() << "WASAPI: GetDefaultAudioEndpoint failed. hr = 0x" << QString::number(hr, 16);
        return false;
    }
    
    IAudioSessionManager2* pSessionManager = NULL;
    hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&pSessionManager);
    pDevice->Release();
    if (FAILED(hr)) {
        qDebug() << "WASAPI: Activate IAudioSessionManager2 failed. hr = 0x" << QString::number(hr, 16);
        return false;
    }
    
    IAudioSessionEnumerator* pSessionEnumerator = NULL;
    hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
    pSessionManager->Release();
    if (FAILED(hr)) {
        qDebug() << "WASAPI: GetSessionEnumerator failed. hr = 0x" << QString::number(hr, 16);
        return false;
    }
    
    int sessionCount = 0;
    pSessionEnumerator->GetCount(&sessionCount);
    
    DWORD currentPid = GetCurrentProcessId();
    
    for (int i = 0; i < sessionCount; i++) {
        IAudioSessionControl* pSessionControl = NULL;
        if (SUCCEEDED(pSessionEnumerator->GetSession(i, &pSessionControl))) {
            IAudioSessionControl2* pSessionControl2 = NULL;
            if (SUCCEEDED(pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2))) {
                DWORD sessionPid = 0;
                if (SUCCEEDED(pSessionControl2->GetProcessId(&sessionPid))) {
                    bool isSystemSounds = (sessionPid == 0 || pSessionControl2->IsSystemSoundsSession() == S_OK);
                    
                    // Look up executable name for sessionPid
                    QString sessionExeName;
                    if (sessionPid != 0 && !isSystemSounds) {
                        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, sessionPid);
                        if (hProcess) {
                            WCHAR szPath[MAX_PATH];
                            if (GetModuleFileNameExW(hProcess, NULL, szPath, MAX_PATH)) {
                                sessionExeName = QFileInfo(QString::fromWCharArray(szPath)).fileName();
                            }
                            CloseHandle(hProcess);
                        }
                    }
                    
                    ISimpleAudioVolume* pSimpleVolume = NULL;
                    if (SUCCEEDED(pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&pSimpleVolume))) {
                        bool shouldMute = false;
                        if (muteActive) {
                            bool isExempt = false;
                            for (const auto &exempt : exemptExes) {
                                if (sessionExeName.compare(exempt, Qt::CaseInsensitive) == 0) {
                                    isExempt = true;
                                    break;
                                }
                            }
                            // Mute if:
                            // - Not an exempt executable name
                            // - Not this soundboard app itself
                            // - Not system sounds (windows beeps)
                            shouldMute = !isExempt && (sessionPid != currentPid) && !isSystemSounds;
                        }
                        
                        hr = pSimpleVolume->SetMute(shouldMute, NULL);
                        if (FAILED(hr)) {
                            qDebug() << "WASAPI: Failed to set mute state for PID" << sessionPid << "to" << shouldMute;
                        }
                        pSimpleVolume->Release();
                    }
                }
                pSessionControl2->Release();
            }
            pSessionControl->Release();
        }
    }
    
    pSessionEnumerator->Release();
    return true;
}

}
}
