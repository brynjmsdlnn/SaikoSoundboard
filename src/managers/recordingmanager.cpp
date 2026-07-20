#include "managers/recordingmanager.h"
#include "logging/LogMacros.h"
#include <QFileInfo>
#include <QDir>
#include <cstring>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

#include "core/adapters/WindowsProcessFinder.h"
#include "core/adapters/WindowsAudioSessionController.h"
#include <QAudioDevice>
#include "audio/wasapipassthrough.h"
#include "models/audiosourcelistmodel.h"

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
        LOG_DEBUG(LogCategory::Recording,
                  QStringLiteral("[RecordingManager] State changed (state: %1)").arg(static_cast<int>(m_state)));
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

void RecordingManager::setSourceModel(AudioSourceListModel *model)
{
    m_sourceModel = model;
    if (m_sourceModel) {
        connect(m_sourceModel, &AudioSourceListModel::hasSourcesChanged,
                this, &RecordingManager::captureReadyChanged);
    }
}

bool RecordingManager::isCaptureModeReady(const QString &mode) const
{
    if (mode == QStringLiteral("global"))
        return true;
    if (mode == QStringLiteral("multi"))
        return m_sourceModel ? m_sourceModel->hasSources() : false;
    return false;
}

void RecordingManager::stopEngine()
{
    LOG_INFO(LogCategory::Recording,
             QStringLiteral("[RecordingManager] Stopping audio recording engine"));
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

    LOG_INFO(LogCategory::Recording,
             QStringLiteral("[RecordingManager] Starting audio recording engine (mode: \"%1\")").arg(mode));

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

    LOG_INFO(LogCategory::Recording,
             QStringLiteral("[RecordingManager] Recording session started (path: \"%1\")").arg(path));

    updateState();
    emit recordingStarted(path);
    return true;
}

void RecordingManager::stopRecording()
{
    QString path = m_wavWriter->fileName();
    m_wavWriter->close();

    LOG_INFO(LogCategory::Recording,
             QStringLiteral("[RecordingManager] Recording session stopped (path: \"%1\")").arg(path));

    if (!m_replayEnabled) {
        stopEngine();
    }

    updateState();
    emit recordingStopped(path);
}

void RecordingManager::setReplayEnabled(bool enabled, const QString &mode)
{
    if (enabled == m_replayEnabled) return;
    LOG_INFO(LogCategory::Recording,
             QStringLiteral("[RecordingManager] Replay buffer %1").arg(enabled ? QStringLiteral("enabled") : QStringLiteral("disabled")));
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
    LOG_DEBUG(LogCategory::Recording,
              QStringLiteral("[RecordingManager] Replay buffer duration set (seconds: %1)").arg(seconds));
    m_replayBuffer->setDuration(seconds);
}

bool RecordingManager::saveReplay(const QString &path)
{
    QByteArray data = m_replayBuffer->getBufferData();
    if (data.isEmpty()) {
        LOG_WARN(LogCategory::Recording,
                 QStringLiteral("[RecordingManager] Replay buffer empty, cannot save (path: \"%1\")").arg(path));
        return false;
    }

    LOG_INFO(LogCategory::Recording,
             QStringLiteral("[RecordingManager] Saving replay buffer (path: \"%1\")").arg(path));

    WAVEFORMATEXTENSIBLE fmt = m_mixer->getOutputFormat();
    if (fmt.Format.nSamplesPerSec == 0) return false;

    WavWriter replayWriter;
    if (!replayWriter.open(path, fmt)) return false;

    replayWriter.writePcm(data);
    replayWriter.close();
    emit replaySaved(path);
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
    LOG_DEBUG(LogCategory::Recording,
              QStringLiteral("[RecordingManager] Source solo status changed (sourceId: \"%1\", soloed: %2)")
                  .arg(sourceId)
                  .arg(solo));
    updateMuteStates();
    emit soloChanged(sourceId, solo);
}

void RecordingManager::setSourceVolume(const QString &sourceId, float volume)
{
    LOG_DEBUG(LogCategory::Recording,
              QStringLiteral("[RecordingManager] Source volume updated (sourceId: \"%1\", volume: %2)")
                  .arg(sourceId)
                  .arg(volume));
    if (m_mixer) {
        m_mixer->updateVolume(sourceId, volume);
    }
    if (m_devicePassthroughs.contains(sourceId)) {
        m_devicePassthroughs[sourceId]->setVolume(volume);
    }
}

void RecordingManager::setSourceMuted(const QString &sourceId, bool muted)
{
    LOG_DEBUG(LogCategory::Recording,
              QStringLiteral("[RecordingManager] Source mute status changed (sourceId: \"%1\", muted: %2)")
                  .arg(sourceId)
                  .arg(muted));
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
