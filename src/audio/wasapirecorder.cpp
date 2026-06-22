#include "audio/wasapirecorder.h"
#include <initguid.h> // Must remain at the top before GUID usage to force local instantiation

#include <QDebug>
#include <QtConcurrent>
#include <QElapsedTimer>
#include <QThread>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <objidl.h>

// Manually define missing modern Windows SDK structures for MinGW compatibility
#ifndef AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK
#define AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK 1

#pragma pack(push, 8)
typedef enum PROCESS_LOOPBACK_MODE {
    PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE = 0,
    PROCESS_LOOPBACK_MODE_EXCLUDE_TARGET_PROCESS_TREE = 1
} PROCESS_LOOPBACK_MODE;

typedef struct AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS {
    DWORD TargetProcessId;
    PROCESS_LOOPBACK_MODE ProcessLoopbackMode;
} AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS;

typedef struct AUDIOCLIENT_ACTIVATION_PARAMS {
    DWORD ActivationType;
    union {
        AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS ProcessLoopbackParams;
    };
} AUDIOCLIENT_ACTIVATION_PARAMS;
#pragma pack(pop)
#endif

#ifndef AUDCLNT_STREAMFLAGS_LOOPBACK
#define AUDCLNT_STREAMFLAGS_LOOPBACK 0x00020000
#endif
#ifndef AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
#define AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM 0x80000000
#endif
#ifndef AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY
#define AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY 0x08000000
#endif

// Define Interface IDs manually for MinGW mapping to resolve linker references
#ifndef __IActivateAudioInterfaceCompletionHandler_INTERFACE_DEFINED__
#define __IActivateAudioInterfaceCompletionHandler_INTERFACE_DEFINED__

// Local instances assigned to prevent missing external reference links
DEFINE_GUID(IID_IAudioClient, 0x1cb9ad4c, 0xdbfa, 0x4c32, 0xb1, 0x78, 0xc2, 0xf5, 0x68, 0xa7, 0x03, 0xb2);
DEFINE_GUID(IID_IActivateAudioInterfaceAsyncOperation, 0x7245b6fb, 0x1d04, 0x4bf0, 0xaa,0xfd, 0xf8,0x29,0x9b,0x7d,0x10,0x11);
DEFINE_GUID(IID_IActivateAudioInterfaceCompletionHandler, 0x9b244110, 0x2649, 0x4bd2, 0x94,0xb1, 0x47,0x2b,0x50,0x3a,0xaf,0x74);
DEFINE_GUID(IID_IAgileObject, 0x94ea2b94, 0xe9cc, 0x49e0, 0xc0,0xff, 0xee,0x64,0xca,0x8f,0x5b,0x90);

struct IActivateAudioInterfaceAsyncOperation : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetActivateResult(HRESULT *activateResult, IUnknown **activatedInterface) = 0;
};

struct IActivateAudioInterfaceCompletionHandler : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE ActivateCompleted(IActivateAudioInterfaceAsyncOperation *activateOperation) = 0;
};
#endif

typedef HRESULT (WINAPI *ActivateAudioInterfaceAsyncPtr)(
    LPCWSTR, REFIID, PROPVARIANT*, IActivateAudioInterfaceCompletionHandler*, IActivateAudioInterfaceAsyncOperation**);

// Apartment-Agile completion handler implementation
class MinGWCompletionHandler : public IActivateAudioInterfaceCompletionHandler, public IAgileObject
{
public:
    MinGWCompletionHandler() : m_ref(1), m_hr(S_OK), m_audioInterface(NULL) {
        m_event = CreateEvent(NULL, TRUE, FALSE, NULL); // manual reset event
    }
    virtual ~MinGWCompletionHandler() {
        if (m_audioInterface) m_audioInterface->Release();
        CloseHandle(m_event);
    }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown || riid == IID_IActivateAudioInterfaceCompletionHandler) {
            *ppv = static_cast<IActivateAudioInterfaceCompletionHandler*>(this);
            AddRef(); return S_OK;
        } else if (riid == IID_IAgileObject) {
            *ppv = static_cast<IAgileObject*>(this);
            AddRef(); return S_OK;
        }
        *ppv = NULL; return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return InterlockedIncrement(&m_ref); }
    STDMETHODIMP_(ULONG) Release() override {
        ULONG ref = InterlockedDecrement(&m_ref);
        if (ref == 0) delete this;
        return ref;
    }
    HRESULT STDMETHODCALLTYPE ActivateCompleted(IActivateAudioInterfaceAsyncOperation *operation) override {
        HRESULT hrActivate = S_OK;
        IUnknown *punkAudioInterface = NULL;
        m_hr = operation->GetActivateResult(&hrActivate, &punkAudioInterface);
        qDebug() << "WASAPI: ActivateCompleted callback. GetActivateResult hr =" << m_hr << "device activation hr =" << hrActivate;
        if (SUCCEEDED(m_hr) && SUCCEEDED(hrActivate)) {
            m_audioInterface = punkAudioInterface;
        } else {
            if (FAILED(hrActivate)) m_hr = hrActivate;
            if (punkAudioInterface) punkAudioInterface->Release();
        }
        SetEvent(m_event);
        return S_OK;
    }

    HANDLE GetEvent() const { return m_event; }
    HRESULT GetResult(IAudioClient **ppClient) {
        if (SUCCEEDED(m_hr) && m_audioInterface) {
            return m_audioInterface->QueryInterface(IID_IAudioClient, (void**)ppClient);
        }
        return m_hr;
    }

private:
    ULONG m_ref;
    HRESULT m_hr;
    HANDLE m_event;
    IUnknown *m_audioInterface;
};

static WAVEFORMATEX* GetSystemMixFormat()
{
    WAVEFORMATEX *pwfx = NULL;
    IMMDeviceEnumerator *pEnum = NULL;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum))) return NULL;
    IMMDevice *pDevice = NULL;
    if (SUCCEEDED(pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice))) {
        IAudioClient *pClient = NULL;
        if (SUCCEEDED(pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pClient))) {
            pClient->GetMixFormat(&pwfx);
            pClient->Release();
        }
        pDevice->Release();
    }
    pEnum->Release();
    return pwfx;
}

int WasapiRecorder::systemMixSampleRate()
{
    WAVEFORMATEX *pwfx = GetSystemMixFormat();
    if (!pwfx) return 48000;
    int rate = static_cast<int>(pwfx->nSamplesPerSec);
    CoTaskMemFree(pwfx);
    return rate;
}

WasapiRecorder::WasapiRecorder(QObject *parent) : QObject(parent), m_processId(0), m_running(false), m_dataChunkOffset(0), m_targetSampleRate(0) {}
WasapiRecorder::~WasapiRecorder() { stop(); }

void WasapiRecorder::setTargetSampleRate(int sampleRate)
{
    m_targetSampleRate = sampleRate;
}

void WasapiRecorder::start(const QString &fileName, DWORD pid)
{
    if (m_running) return;
    m_fileName = fileName;
    m_processId = pid;
    m_running = true;
    m_future = QtConcurrent::run([this]() { runCapture(); });
}

void WasapiRecorder::start(DWORD pid)
{
    if (m_running) return;
    m_fileName.clear();
    m_processId = pid;
    m_running = true;
    m_future = QtConcurrent::run([this]() { runCapture(); });
}

void WasapiRecorder::stop()
{
    m_running = false;
    if (m_future.isRunning())
        m_future.waitForFinished();
}

struct COMInitializer {
    HRESULT hr;
    COMInitializer() {
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    }
    ~COMInitializer() {
        if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) {
            CoUninitialize();
        }
    }
    bool isValid() const { return SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE; }
};

void WasapiRecorder::runCapture()
{
    COMInitializer comInit;
    if (!comInit.isValid()) {
        emit error("COM initialization failed.");
        return;
    }

    IAudioClient *pAudioClient = NULL;
    DWORD flags = AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

    if (m_processId != 0) {
        qDebug() << "WASAPI: Attempting per-process audio capture for PID:" << m_processId;
        HMODULE hLib = LoadLibraryA("mmdevapi.dll");
        if (hLib) {
            auto pActivateFn = (ActivateAudioInterfaceAsyncPtr)GetProcAddress(hLib, "ActivateAudioInterfaceAsync");
            if (pActivateFn) {
                AUDIOCLIENT_ACTIVATION_PARAMS params = {0};
                params.ActivationType = AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
                params.ProcessLoopbackParams.TargetProcessId = m_processId;
                params.ProcessLoopbackParams.ProcessLoopbackMode = PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;

                PROPVARIANT prop;
                PropVariantInit(&prop);
                prop.vt = VT_BLOB;
                prop.blob.cbSize = sizeof(params);
                prop.blob.pBlobData = (BYTE*)&params;

                MinGWCompletionHandler *handler = new MinGWCompletionHandler();
                IActivateAudioInterfaceAsyncOperation *operation = NULL;

                // BUG FIX 1: Pass IID_IAudioClient instead of IID_IActivateAudioInterfaceCompletionHandler
                HRESULT hr = pActivateFn(L"VAD\\Process_Loopback", IID_IAudioClient, &prop, handler, &operation);
                if (SUCCEEDED(hr)) {
                    qDebug() << "WASAPI: ActivateAudioInterfaceAsync operation started successfully. Waiting for callback...";
                    if (WaitForSingleObject(handler->GetEvent(), 5000) == WAIT_OBJECT_0) {
                        HRESULT hrResult = handler->GetResult(&pAudioClient);
                        if (SUCCEEDED(hrResult)) {
                            qDebug() << "WASAPI: Per-process Audio Client successfully activated.";
                            // Keep loopback flags!
                        } else {
                            qDebug() << "WASAPI: Failed to get activated Audio Client interface. hr =" << hrResult;
                        }
                    } else {
                        qDebug() << "WASAPI: Timeout waiting for per-process activation callback.";
                    }
                    operation->Release();
                } else {
                    qDebug() << "WASAPI: ActivateAudioInterfaceAsync failed with hr =" << hr;
                }
                handler->Release();
            } else {
                qDebug() << "WASAPI: Failed to get proc address of ActivateAudioInterfaceAsync.";
            }
            FreeLibrary(hLib);
        } else {
            qDebug() << "WASAPI: Failed to load mmdevapi.dll.";
        }
    }

    // System-wide loopback fallback
    if (!pAudioClient) {
        qDebug() << "WASAPI: Falling back to system-wide loopback recording...";
        IMMDeviceEnumerator *pEnum = NULL;
        if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum))) {
            IMMDevice *pDevice = NULL;
            if (SUCCEEDED(pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice))) {
                pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
                pDevice->Release();
            }
            pEnum->Release();
        }
    }

    if (!pAudioClient) {
        emit error("Could not initialize windows audio sub-system client.");
        return;
    }

    WAVEFORMATEX *pwfx = GetSystemMixFormat();
    if (!pwfx) {
        pAudioClient->Release();
        return;
    }

    if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        memcpy(&m_format, pwfx, sizeof(WAVEFORMATEXTENSIBLE));
    } else {
        memset(&m_format, 0, sizeof(WAVEFORMATEXTENSIBLE));
        memcpy(&m_format, pwfx, sizeof(WAVEFORMATEX) + pwfx->cbSize);
    }

    // Override format with user-requested sample rate if set
    WAVEFORMATEX *pInitFormat = pwfx;
    WAVEFORMATEXTENSIBLE customFormat;
    memset(&customFormat, 0, sizeof(customFormat));

    if (m_targetSampleRate > 0) {
        // Build a custom float format at the desired sample rate.
        // WASAPI handles sample rate conversion via AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM.
        customFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        customFormat.Format.nChannels = m_format.Format.nChannels;
        customFormat.Format.nSamplesPerSec = m_targetSampleRate;
        customFormat.Format.wBitsPerSample = 32;
        customFormat.Format.nBlockAlign = customFormat.Format.nChannels * 4;
        customFormat.Format.nAvgBytesPerSec = customFormat.Format.nSamplesPerSec * customFormat.Format.nBlockAlign;
        customFormat.Format.cbSize = 22;
        customFormat.Samples.wValidBitsPerSample = 32;
        customFormat.dwChannelMask = m_format.dwChannelMask;
        customFormat.SubFormat = m_format.SubFormat;

        // If system mix isn't float, default to IEEE float subformat
        if (customFormat.SubFormat.Data1 == 0) {
            customFormat.SubFormat = {0x00000003, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
        }

        memcpy(&m_format, &customFormat, sizeof(WAVEFORMATEXTENSIBLE));
        pInitFormat = (WAVEFORMATEX*)&m_format;
    }

    REFERENCE_TIME bufDuration = 0; // Set to 0 to use engine default buffer duration and avoid alignment issues

    qDebug() << "WASAPI: Initializing audio client. ShareMode: Shared, Flags: 0x" << QString::number(flags, 16);
    HRESULT hrInit = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, bufDuration, 0, pInitFormat, NULL);
    if (FAILED(hrInit)) {
        qDebug() << "WASAPI: Initialize with autoconvert flags failed (hr = 0x" << QString::number(hrInit, 16) << "). Retrying with basic loopback flag...";
        flags = AUDCLNT_STREAMFLAGS_LOOPBACK;
        hrInit = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, bufDuration, 0, pInitFormat, NULL);
    }

    if (FAILED(hrInit)) {
        qDebug() << "WASAPI: pAudioClient->Initialize failed with hr = 0x" << QString::number(hrInit, 16);
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        emit error(QString("Initialization of audio stream failed. hr = 0x%1").arg(hrInit, 8, 16, QChar('0')));
        return;
    }

    IAudioCaptureClient *pCaptureClient = NULL;
    if (FAILED(pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient))) {
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        return;
    }

    pAudioClient->Start();
    
    QFile file;
    bool writingToFile = !m_fileName.isEmpty();
    if (writingToFile) {
        if (file.open(QIODevice::WriteOnly)) {
            writeWavHeader(file, pInitFormat);
        } else {
            writingToFile = false;
        }
    }

    QElapsedTimer timer;
    timer.start();
    qint64 totalBytes = 0;

    while (m_running) {
        UINT32 packetLength = 0;
        pCaptureClient->GetNextPacketSize(&packetLength);
        while (packetLength != 0) {
            BYTE *pData;
            UINT32 numFramesRead;
            DWORD bufFlags;
            if (SUCCEEDED(pCaptureClient->GetBuffer(&pData, &numFramesRead, &bufFlags, NULL, NULL))) {
                qint64 bytesCaptured = numFramesRead * pInitFormat->nBlockAlign;
                
                if (!(bufFlags & 0x2)) {
                    if (writingToFile) {
                        totalBytes += file.write((const char*)pData, bytesCaptured);
                    }
                    
                    // Emit raw PCM data for real-time mixing
                    QByteArray pcmChunk((const char*)pData, bytesCaptured);
                    emit pcmDataReady(pcmChunk);
                } else {
                    // Silent frame, but still emit empty data if needed for timing? 
                    // For now just skip, as mixer will handle gaps via timing/buffering.
                }
                pCaptureClient->ReleaseBuffer(numFramesRead);
            }
            pCaptureClient->GetNextPacketSize(&packetLength);
        }
        emit statsUpdated(totalBytes, timer.elapsed() / 1000.0);
        QThread::msleep(10);
    }
    
    pAudioClient->Stop();
    if (writingToFile) {
        updateWavHeader(file);
        file.close();
    }

    pCaptureClient->Release();
    CoTaskMemFree(pwfx);
    pAudioClient->Release();
    emit finished();
}

bool WasapiRecorder::writeWavHeader(QFile &file, const void* pwfx)
{
    const WAVEFORMATEX* w = (const WAVEFORMATEX*)pwfx;
    file.write("RIFF", 4);
    quint32 fileSize = 0;
    file.write((const char*)&fileSize, 4);
    file.write("WAVE", 4);
    file.write("fmt ", 4);
    
    // fmt chunk size: 16 for PCM, 40 for EX, or 18 + cbSize for others
    quint32 fmtSize = 16;
    if (w->wFormatTag == WAVE_FORMAT_EXTENSIBLE) fmtSize = 40;
    else if (w->wFormatTag != WAVE_FORMAT_PCM) fmtSize = 18 + w->cbSize;
    
    file.write((const char*)&fmtSize, 4);
    file.write((const char*)w, fmtSize);
    file.write("data", 4);
    m_dataChunkOffset = file.pos(); // save the offset where dataSize is written
    quint32 dataSize = 0;
    file.write((const char*)&dataSize, 4);
    return file.error() == QFile::NoError;
}

void WasapiRecorder::updateWavHeader(QFile &file)
{
    if (m_dataChunkOffset <= 0 || file.size() < (m_dataChunkOffset + 4)) return;
    quint32 fileSize = (quint32)(file.size() - 8);
    file.seek(4);
    file.write((const char*)&fileSize, 4);
    
    file.seek(m_dataChunkOffset);
    quint32 dataSize = (quint32)(file.size() - (m_dataChunkOffset + 4));
    file.write((const char*)&dataSize, 4);
}