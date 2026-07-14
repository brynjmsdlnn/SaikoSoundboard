#include "audio/wasapipassthrough.h"
#include <initguid.h>

#include <QtConcurrent>
#include <QThread>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <objidl.h>
#include <mmreg.h>
#include <propsys.h>
#include <propidl.h>

#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdint>

#ifndef AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
#define AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM 0x80000000
#endif
#ifndef AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY
#define AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY 0x08000000
#endif

// PKEY_Device_FriendlyName from functiondiscoverykeys.h (not available in MinGW)
static const PROPERTYKEY PKEY_Device_FriendlyName_W = { { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 14 };

struct PassthroughCOMInitializer {
    HRESULT hr;
    PassthroughCOMInitializer() { hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); }
    ~PassthroughCOMInitializer() {
        if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) CoUninitialize();
    }
    bool isValid() const { return SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE; }
};

static QString getDeviceFriendlyName(IMMDevice *device)
{
    IPropertyStore *props = nullptr;
    if (FAILED(device->OpenPropertyStore(STGM_READ, &props)))
        return {};
    PROPVARIANT var;
    PropVariantInit(&var);
    QString name;
    if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName_W, &var)) && var.vt == VT_LPWSTR)
        name = QString::fromWCharArray(var.pwszVal);
    PropVariantClear(&var);
    props->Release();
    return name;
}

static IMMDevice *findDeviceByDesc(EDataFlow flow, const QString &desc)
{
    IMMDeviceEnumerator *pEnum = nullptr;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                __uuidof(IMMDeviceEnumerator), (void**)&pEnum)))
        return nullptr;

    IMMDeviceCollection *pCol = nullptr;
    if (FAILED(pEnum->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pCol))) {
        pEnum->Release();
        return nullptr;
    }

    IMMDevice *result = nullptr;
    UINT count = 0;
    pCol->GetCount(&count);
    for (UINT i = 0; i < count; ++i) {
        IMMDevice *pDev = nullptr;
        if (SUCCEEDED(pCol->Item(i, &pDev))) {
            QString name = getDeviceFriendlyName(pDev);
            if (name == desc) {
                result = pDev;
                break;
            }
            pDev->Release();
        }
    }
    pCol->Release();
    pEnum->Release();
    return result;
}

static bool isFloatFormat(const WAVEFORMATEX *pwfx)
{
    if (!pwfx) return false;
    if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) return true;
    if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE && pwfx->cbSize >= 22) {
        const WAVEFORMATEXTENSIBLE *ext = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(pwfx);
        static const GUID guidFloat = { 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
        return ext->SubFormat == guidFloat;
    }
    return false;
}

static WORD getBitsPerSample(const WAVEFORMATEX *pwfx)
{
    if (!pwfx) return 0;
    if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE && pwfx->cbSize >= 22) {
        const WAVEFORMATEXTENSIBLE *ext = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(pwfx);
        if (ext->Samples.wValidBitsPerSample > 0)
            return ext->Samples.wValidBitsPerSample;
    }
    return pwfx->wBitsPerSample;
}

static float readInputSample(const BYTE *src, bool isFloat, WORD bitsPerSample)
{
    if (isFloat) {
        return *reinterpret_cast<const float*>(src);
    }
    if (bitsPerSample == 16) {
        int16_t val = *reinterpret_cast<const int16_t*>(src);
        return val / 32768.0f;
    }
    if (bitsPerSample == 24) {
        int32_t val = (src[0]) | (src[1] << 8) | (static_cast<int8_t>(src[2]) << 16);
        return val / 8388608.0f;
    }
    if (bitsPerSample == 32) {
        int32_t val = *reinterpret_cast<const int32_t*>(src);
        return val / 2147483648.0f;
    }
    if (bitsPerSample == 8) {
        return (*src - 128) / 128.0f;
    }
    return 0.0f;
}

static void writeOutputSample(BYTE *dst, float sample, bool isFloat, WORD bitsPerSample)
{
    sample = std::clamp(sample, -1.0f, 1.0f);
    if (isFloat) {
        *reinterpret_cast<float*>(dst) = sample;
        return;
    }
    if (bitsPerSample == 16) {
        *reinterpret_cast<int16_t*>(dst) = static_cast<int16_t>(std::clamp(sample * 32767.0f, -32768.0f, 32767.0f));
        return;
    }
    if (bitsPerSample == 24) {
        int32_t val = static_cast<int32_t>(std::clamp(sample * 8388607.0f, -8388608.0f, 8388607.0f));
        dst[0] = static_cast<BYTE>(val & 0xFF);
        dst[1] = static_cast<BYTE>((val >> 8) & 0xFF);
        dst[2] = static_cast<BYTE>((val >> 16) & 0xFF);
        return;
    }
    if (bitsPerSample == 32) {
        *reinterpret_cast<int32_t*>(dst) = static_cast<int32_t>(std::clamp(sample * 2147483647.0f, -2147483648.0f, 2147483647.0f));
        return;
    }
}

WasapiPassthrough::WasapiPassthrough(QObject *parent) : QObject(parent), m_running(false), m_volume(1.0f) {}
WasapiPassthrough::~WasapiPassthrough() { stop(); }

void WasapiPassthrough::setVolume(float volume)
{
    m_volume.store(volume);
}

void WasapiPassthrough::start(const QString &inputDeviceDesc, const QString &outputDeviceDesc)
{
    if (m_running) return;
    m_running = true;
    m_future = QtConcurrent::run([this, inputDeviceDesc, outputDeviceDesc]() {
        runPassthrough(inputDeviceDesc, outputDeviceDesc);
    });
}

void WasapiPassthrough::stop()
{
    m_running = false;
    if (m_future.isRunning())
        m_future.waitForFinished();
}

void WasapiPassthrough::runPassthrough(const QString &inputDeviceDesc, const QString &outputDeviceDesc)
{
    PassthroughCOMInitializer comInit;
    if (!comInit.isValid()) {
        emit error("COM initialization failed.");
        return;
    }

    IMMDevice *pInDevice = findDeviceByDesc(eCapture, inputDeviceDesc);
    IMMDevice *pOutDevice = findDeviceByDesc(eRender, outputDeviceDesc);

    if (!pInDevice && !inputDeviceDesc.isEmpty())
        pInDevice = findDeviceByDesc(eCapture, {});
    if (!pInDevice) {
        IMMDeviceEnumerator *pEnum = nullptr;
        if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                       __uuidof(IMMDeviceEnumerator), (void**)&pEnum))) {
            pEnum->GetDefaultAudioEndpoint(eCapture, eConsole, &pInDevice);
            pEnum->Release();
        }
    }
    if (!pOutDevice && !outputDeviceDesc.isEmpty())
        pOutDevice = findDeviceByDesc(eRender, {});
    if (!pOutDevice) {
        IMMDeviceEnumerator *pEnum = nullptr;
        if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                       __uuidof(IMMDeviceEnumerator), (void**)&pEnum))) {
            pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pOutDevice);
            pEnum->Release();
        }
    }

    if (!pInDevice || !pOutDevice) {
        emit error("Could not find input or output audio device.");
        if (pInDevice) pInDevice->Release();
        if (pOutDevice) pOutDevice->Release();
        return;
    }

    IAudioClient *pInClient = nullptr;
    IAudioClient *pOutClient = nullptr;
    WAVEFORMATEX *pwfxIn = nullptr;
    WAVEFORMATEX *pwfxOut = nullptr;

    HRESULT hr = pInDevice->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&pInClient);
    if (SUCCEEDED(hr))
        hr = pOutDevice->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&pOutClient);
    if (SUCCEEDED(hr))
        hr = pInClient->GetMixFormat(&pwfxIn);
    if (SUCCEEDED(hr))
        hr = pOutClient->GetMixFormat(&pwfxOut);

    if (FAILED(hr) || !pwfxIn || !pwfxOut) {
        emit error("Failed to initialize audio clients.");
        if (pwfxIn) CoTaskMemFree(pwfxIn);
        if (pwfxOut) CoTaskMemFree(pwfxOut);
        if (pInClient) pInClient->Release();
        if (pOutClient) pOutClient->Release();
        pInDevice->Release();
        pOutDevice->Release();
        return;
    }

    REFERENCE_TIME bufDuration = 200000; // 20ms buffer duration

    DWORD flags = AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;
    hr = pInClient->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, bufDuration, 0, pwfxIn, nullptr);
    if (FAILED(hr)) {
        hr = pInClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, bufDuration, 0, pwfxIn, nullptr);
    }

    HRESULT hrOut = pOutClient->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, bufDuration, 0, pwfxOut, nullptr);
    if (FAILED(hrOut)) {
        hrOut = pOutClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, bufDuration, 0, pwfxOut, nullptr);
    }

    if (FAILED(hr) || FAILED(hrOut)) {
        emit error("Failed to initialize audio stream.");
        CoTaskMemFree(pwfxIn);
        CoTaskMemFree(pwfxOut);
        pInClient->Release();
        pOutClient->Release();
        pInDevice->Release();
        pOutDevice->Release();
        return;
    }

    IAudioCaptureClient *pCapture = nullptr;
    IAudioRenderClient *pRender = nullptr;
    UINT32 outBufferFrameCount = 0;

    hr = pInClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCapture);
    if (SUCCEEDED(hr))
        hr = pOutClient->GetService(__uuidof(IAudioRenderClient), (void**)&pRender);
    if (SUCCEEDED(hr))
        hr = pOutClient->GetBufferSize(&outBufferFrameCount);

    if (FAILED(hr) || !pCapture || !pRender) {
        emit error("Failed to get audio services.");
        if (pCapture) pCapture->Release();
        if (pRender) pRender->Release();
        CoTaskMemFree(pwfxIn);
        CoTaskMemFree(pwfxOut);
        pInClient->Release();
        pOutClient->Release();
        pInDevice->Release();
        pOutDevice->Release();
        return;
    }

    pInClient->Start();
    pOutClient->Start();

    const bool isInFloat = isFloatFormat(pwfxIn);
    const bool isOutFloat = isFloatFormat(pwfxOut);
    const WORD inBitsPerSample = getBitsPerSample(pwfxIn);
    const WORD outBitsPerSample = getBitsPerSample(pwfxOut);
    const WORD inChannels = pwfxIn->nChannels;
    const WORD outChannels = pwfxOut->nChannels;
    const DWORD inRate = pwfxIn->nSamplesPerSec;
    const DWORD outRate = pwfxOut->nSamplesPerSec;
    const UINT32 inBlockAlign = pwfxIn->nBlockAlign;
    const UINT32 inBytesPerSample = (inBitsPerSample / 8);
    const UINT32 outBytesPerSample = (outBitsPerSample / 8);

    std::vector<float> fifoBuffer;
    fifoBuffer.reserve(outRate * outChannels / 5); // Reserve ~200ms space

    std::vector<float> tempConvertedFrames;
    double resamplePos = 0.0;

    while (m_running) {
        // --- 1. CAPTURE DATA FROM INPUT CLIENT ---
        UINT32 packetLength = 0;
        pCapture->GetNextPacketSize(&packetLength);

        while (packetLength > 0 && m_running) {
            BYTE *pData = nullptr;
            UINT32 numFramesRead = 0;
            DWORD bufFlags = 0;

            if (SUCCEEDED(pCapture->GetBuffer(&pData, &numFramesRead, &bufFlags, nullptr, nullptr))) {
                if (numFramesRead > 0) {
                    tempConvertedFrames.resize(numFramesRead * outChannels);
                    bool isSilent = (bufFlags & AUDCLNT_BUFFERFLAGS_SILENT) || (pData == nullptr);

                    float vol = m_volume.load();
                    for (UINT32 i = 0; i < numFramesRead; ++i) {
                        float inChSamples[8] = {0.0f};
                        if (!isSilent) {
                            for (WORD c = 0; c < std::min(inChannels, static_cast<WORD>(8)); ++c) {
                                const BYTE *samplePtr = pData + (i * inBlockAlign) + (c * inBytesPerSample);
                                inChSamples[c] = readInputSample(samplePtr, isInFloat, inBitsPerSample) * vol;
                            }
                        }

                        // Map input channels to output channels
                        if (inChannels == 1 && outChannels >= 2) {
                            tempConvertedFrames[i * outChannels + 0] = inChSamples[0];
                            tempConvertedFrames[i * outChannels + 1] = inChSamples[0];
                            for (WORD c = 2; c < outChannels; ++c) {
                                tempConvertedFrames[i * outChannels + c] = 0.0f;
                            }
                        } else if (inChannels >= 2 && outChannels == 1) {
                            tempConvertedFrames[i * outChannels + 0] = 0.5f * (inChSamples[0] + inChSamples[1]);
                        } else {
                            for (WORD c = 0; c < outChannels; ++c) {
                                tempConvertedFrames[i * outChannels + c] = (c < inChannels) ? inChSamples[c] : 0.0f;
                            }
                        }
                    }

                    // Resample to output rate if necessary
                    if (inRate == outRate) {
                        fifoBuffer.insert(fifoBuffer.end(), tempConvertedFrames.begin(), tempConvertedFrames.end());
                    } else {
                        double ratio = static_cast<double>(inRate) / static_cast<double>(outRate);
                        while (resamplePos < static_cast<double>(numFramesRead)) {
                            size_t idx0 = static_cast<size_t>(resamplePos);
                            size_t idx1 = std::min(idx0 + 1, static_cast<size_t>(numFramesRead - 1));
                            float frac = static_cast<float>(resamplePos - idx0);

                            for (WORD c = 0; c < outChannels; ++c) {
                                float s0 = tempConvertedFrames[idx0 * outChannels + c];
                                float s1 = tempConvertedFrames[idx1 * outChannels + c];
                                fifoBuffer.push_back(s0 + frac * (s1 - s0));
                            }
                            resamplePos += ratio;
                        }
                        resamplePos -= numFramesRead;
                    }
                }
                pCapture->ReleaseBuffer(numFramesRead);
            }
            pCapture->GetNextPacketSize(&packetLength);
        }

        // --- 2. RENDER DATA TO OUTPUT CLIENT ---
        UINT32 numPaddingFrames = 0;
        if (SUCCEEDED(pOutClient->GetCurrentPadding(&numPaddingFrames))) {
            UINT32 availableFrames = (outBufferFrameCount > numPaddingFrames) ? (outBufferFrameCount - numPaddingFrames) : 0;
            UINT32 fifoFrames = static_cast<UINT32>(fifoBuffer.size() / outChannels);
            UINT32 framesToWrite = std::min(availableFrames, fifoFrames);

            if (framesToWrite > 0) {
                BYTE *pRenderData = nullptr;
                if (SUCCEEDED(pRender->GetBuffer(framesToWrite, &pRenderData))) {
                    for (UINT32 i = 0; i < framesToWrite; ++i) {
                        for (WORD c = 0; c < outChannels; ++c) {
                            float sample = fifoBuffer[i * outChannels + c];
                            BYTE *dstPtr = pRenderData + (i * pwfxOut->nBlockAlign) + (c * outBytesPerSample);
                            writeOutputSample(dstPtr, sample, isOutFloat, outBitsPerSample);
                        }
                    }
                    pRender->ReleaseBuffer(framesToWrite, 0);
                    fifoBuffer.erase(fifoBuffer.begin(), fifoBuffer.begin() + (framesToWrite * outChannels));
                }
            }
        }

        // Limit FIFO buffer size to prevent memory growth if output freezes
        size_t maxFifoSamples = static_cast<size_t>(outChannels) * (outRate / 2); // 0.5s max
        if (fifoBuffer.size() > maxFifoSamples) {
            fifoBuffer.erase(fifoBuffer.begin(), fifoBuffer.end() - maxFifoSamples);
        }

        QThread::msleep(2);
    }

    pInClient->Stop();
    pOutClient->Stop();

    pCapture->Release();
    pRender->Release();
    CoTaskMemFree(pwfxIn);
    CoTaskMemFree(pwfxOut);
    pInClient->Release();
    pOutClient->Release();
    pInDevice->Release();
    pOutDevice->Release();
}
