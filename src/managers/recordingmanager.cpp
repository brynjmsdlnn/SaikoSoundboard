#include "managers/recordingmanager.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <cstring>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

#include "core/adapters/WindowsProcessFinder.h"
#include "core/adapters/WindowsAudioSessionController.h"
#include <QAudioDevice>
#include "audio/wasapipassthrough.h"

static QString mapRenderToCaptureDevice(const QString &renderName)
{
    QString captureName = renderName;
    if (captureName.contains("Input")) {
        captureName.replace("Input", "Output");
    } else if (captureName.contains("input")) {
        captureName.replace("input", "output");
    }
    return captureName;
}

RecordingManager::RecordingManager(SettingsManager *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_replayEnabled(false)
    , m_state(CaptureState::Idle)
{
    m_mixer = new AudioMixer(this);
    m_replayBuffer = new ReplayBuffer(this);
    m_replayBuffer->setDuration(m_settings->replayDuration());
    m_wavWriter = new WavWriter(this);

    connect(m_mixer, &AudioMixer::mixedPcmReady, this, [this](const QByteArray &data){
        if (m_wavWriter->isOpen()) {
            m_wavWriter->writePcm(data);
        }
        if (m_replayEnabled) {
            m_replayBuffer->pushPcmChunk(data);
        }
    });

    connect(m_settings, &SettingsManager::sourcesChanged, this, &RecordingManager::syncDevicePassthroughs);
    connect(m_settings, &SettingsManager::localMonitorDeviceChanged, this, &RecordingManager::syncDevicePassthroughs);

    syncDevicePassthroughs();
}

RecordingManager::~RecordingManager()
{
    stopEngine();
    for (auto passthrough : std::as_const(m_devicePassthroughs)) {
        passthrough->stop();
        delete passthrough;
    }
    m_devicePassthroughs.clear();
}

void RecordingManager::updateState()
{
    bool prevEngine = !m_activeRecorders.isEmpty();
    bool prevRecording = m_wavWriter->isOpen();

    CaptureState newState;
    bool recording = isRecording();

    if (m_replayEnabled && recording) {
        newState = CaptureState::RecordingAndReplay;
    } else if (m_replayEnabled) {
        newState = CaptureState::ReplayOnly;
    } else if (recording) {
        newState = CaptureState::Recording;
    } else {
        newState = CaptureState::Idle;
    }

    if (newState != m_state) {
        m_state = newState;
        emit stateChanged(m_state);
        qDebug() << "RecordingManager: State changed to" << (int)m_state;
    }

    bool newEngine = !m_activeRecorders.isEmpty();
    if (newEngine != prevEngine) {
        emit engineRunningChanged();
    }
    bool newRecording = m_wavWriter->isOpen();
    if (newRecording != prevRecording) {
        emit recordingChanged();
    }
}

void RecordingManager::stopEngine()
{
    m_mixer->stop();
    
    // Restore system volumes for all tracked PIDs
    Saiko::Adapters::WindowsAudioSessionController::setAbsoluteMuteExcept({}, false);
    m_sourcePids.clear();
    
    for (auto rec : std::as_const(m_activeRecorders)) {
        rec->stop();
    }
    m_activeRecorders.clear();

    WAVEFORMATEXTENSIBLE format;
    memset(&format, 0, sizeof(format));
    m_mixer->setOutputFormat(format);
    m_replayBuffer->setFormat(format);

    updateState();
    emit engineStopped();
}

void RecordingManager::startEngine(const QString &mode)
{
    if (isEngineRunning()) return;

    WAVEFORMATEXTENSIBLE format;
    memset(&format, 0, sizeof(format));
    m_mixer->setOutputFormat(format);
    m_mixer->start();

    m_soloedSources.clear();
    m_sourcePids.clear();
    QList<AudioSource> sources = m_settings->sources();
    for (const auto &src : std::as_const(sources)) {
        if (src.enabled && src.solo) {
            m_soloedSources.insert(src.id);
        }
    }

    if (mode == "global") {
        startDeviceRecorder("", "global", 1.0f);
    } else {
        QList<AudioSource> sources = m_settings->sources();
        for (const auto& src : std::as_const(sources)) {
            if (src.type == "device") {
                startDeviceRecorder(src.deviceName, src.id, src.volume);
                m_mixer->setSourceMuted(src.id, !src.enabled);
            } else {
                DWORD pid = Saiko::Adapters::WindowsProcessFinder::findProcessId(src.executableName);
                if (pid != 0) {
                    startProcessRecorder(pid, src.id, src.volume);
                    m_mixer->setSourceMuted(src.id, !src.enabled);
                }
            }
        }
    }
    updateState();
    emit engineStarted();
}

bool RecordingManager::startRecording(const QString &path)
{
    WAVEFORMATEXTENSIBLE fmt = m_mixer->getOutputFormat();
    if (!m_wavWriter->open(path, fmt)) {
        return false;
    }

    updateState();
    emit recordingStarted(path);
    return true;
}

void RecordingManager::stopRecording()
{
    QString path = m_wavWriter->fileName();
    m_wavWriter->close();

    if (!m_replayEnabled) {
        stopEngine();
    }

    updateState();
    emit recordingStopped(path);
}

void RecordingManager::setReplayEnabled(bool enabled, const QString &mode)
{
    if (enabled == m_replayEnabled) return;
    m_replayEnabled = enabled;
    if (enabled) {
        if (!isEngineRunning()) {
            startEngine(mode);
        }
    } else {
        m_replayBuffer->clear();
        if (!isRecording()) {
            stopEngine();
        }
    }
    updateState();
    emit replayActiveChanged();
}

void RecordingManager::setReplayDuration(int seconds)
{
    m_replayBuffer->setDuration(seconds);
}

bool RecordingManager::saveReplay(const QString &path)
{
    QByteArray data = m_replayBuffer->getBufferData();
    if (data.isEmpty()) return false;

    WAVEFORMATEXTENSIBLE fmt = m_mixer->getOutputFormat();
    if (fmt.Format.nSamplesPerSec == 0) return false;

    WavWriter replayWriter;
    if (!replayWriter.open(path, fmt)) return false;

    replayWriter.writePcm(data);
    replayWriter.close();
    return true;
}

void RecordingManager::startProcessRecorder(DWORD pid, const QString& sourceId, float volume)
{
    WasapiRecorder *rec = new WasapiRecorder(this);
    setupRecorder(rec, sourceId, volume);

    rec->start(pid, "");
    m_activeRecorders.append(rec);

    m_sourcePids.insert(sourceId, pid);
    updateMuteStates();
}

void RecordingManager::startDeviceRecorder(const QString& deviceName, const QString& sourceId, float volume)
{
    WasapiRecorder *rec = new WasapiRecorder(this);
    setupRecorder(rec, sourceId, volume);

    rec->start(0, deviceName);
    m_activeRecorders.append(rec);

    m_sourcePids.insert(sourceId, 0);
    updateMuteStates();
}

void RecordingManager::setupRecorder(WasapiRecorder *rec, const QString& sourceId, float volume)
{
    rec->setTargetSampleRate(m_settings->recordingSampleRate());
    m_mixer->addSource(sourceId, volume);

    connect(rec, &WasapiRecorder::pcmDataReady, this, [this, sourceId](const QByteArray &data){
        m_mixer->pushPcmData(sourceId, data);
    });

    connect(rec, &WasapiRecorder::finished, rec, &QObject::deleteLater);
    connect(rec, &WasapiRecorder::error, this, &RecordingManager::errorOccurred);

    connect(rec, &WasapiRecorder::statsUpdated, this, [this, rec](qint64, double){
        if (m_mixer->getOutputFormat().Format.nSamplesPerSec == 0) {
            WAVEFORMATEXTENSIBLE fmt = rec->getFormat();
            m_mixer->setOutputFormat(fmt);
            m_replayBuffer->setFormat(fmt);

            if (m_wavWriter->isOpen() && m_wavWriter->size() == 0) {
                m_wavWriter->open(m_wavWriter->fileName(), fmt);
            }
        }
    });
}

void RecordingManager::updateMuteStates()
{
    QSet<QString> exemptExes;
    QList<AudioSource> sources = m_settings->sources();
    for (const auto &srcId : std::as_const(m_soloedSources)) {
        for (const auto &src : std::as_const(sources)) {
            if (src.id == srcId) {
                exemptExes.insert(src.executableName);
                break;
            }
        }
    }

    bool muteActive = !m_soloedSources.isEmpty();
    Saiko::Adapters::WindowsAudioSessionController::setAbsoluteMuteExcept(exemptExes, muteActive);
}

void RecordingManager::setSourceSolo(const QString &sourceId, bool solo)
{
    if (solo) {
        m_soloedSources.insert(sourceId);
    } else {
        m_soloedSources.remove(sourceId);
    }
    updateMuteStates();
    emit soloChanged(sourceId, solo);
}

void RecordingManager::setSourceVolume(const QString &sourceId, float volume)
{
    if (m_mixer) {
        m_mixer->updateVolume(sourceId, volume);
    }
    if (m_devicePassthroughs.contains(sourceId)) {
        m_devicePassthroughs[sourceId]->setVolume(volume);
    }
}

void RecordingManager::setSourceMuted(const QString &sourceId, bool muted)
{
    if (m_mixer) {
        m_mixer->setSourceMuted(sourceId, muted);
    }
}

void RecordingManager::syncDevicePassthroughs()
{
    // Tear down all active passthroughs
    for (auto passthrough : std::as_const(m_devicePassthroughs)) {
        passthrough->stop();
        passthrough->deleteLater();
    }
    m_devicePassthroughs.clear();
    
    // Rebuild active passthroughs for all monitored device sources
    QList<AudioSource> sources = m_settings->sources();
    for (const auto &src : std::as_const(sources)) {
        if (src.type == "device" && src.monitor) {
            QString captureDev = mapRenderToCaptureDevice(src.deviceName);
            WasapiPassthrough *passthrough = new WasapiPassthrough(this);
            passthrough->start(captureDev, m_settings->localMonitorDevice());
            passthrough->setVolume(src.volume);
            m_devicePassthroughs.insert(src.id, passthrough);
        }
    }
}
