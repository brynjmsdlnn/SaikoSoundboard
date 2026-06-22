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
}

RecordingManager::~RecordingManager()
{
    stopEngine();
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

    if (mode == "global") {
        startRecorderForPid(0, "global", 1.0f);
    } else {
        QList<AudioSource> sources = m_settings->sources();
        for (const auto& src : std::as_const(sources)) {
            if (!src.enabled) continue;

            DWORD pid = Saiko::Adapters::WindowsProcessFinder::findProcessId(src.executableName);
            if (pid != 0) {
                startRecorderForPid(pid, src.id, src.volume);
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

void RecordingManager::startRecorderForPid(DWORD pid, const QString& sourceId, float volume)
{
    WasapiRecorder *rec = new WasapiRecorder(this);
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

    rec->start(pid);
    m_activeRecorders.append(rec);
}
