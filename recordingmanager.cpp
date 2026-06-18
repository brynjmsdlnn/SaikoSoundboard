#include "recordingmanager.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

RecordingManager::RecordingManager(SettingsManager *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_replayEnabled(false)
{
    m_mixer = new AudioMixer(this);
    m_replayBuffer = new ReplayBuffer(this);
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
                                if (exeName.compare(src.executableName, Qt::CaseInsensitive) == 0) {
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
            if (pid != 0) {
                startRecorderForPid(pid, src.id, src.volume);
            }
        }
    }
    emit engineStarted();
}

bool RecordingManager::startRecording(const QString &path)
{
    WAVEFORMATEXTENSIBLE fmt = m_mixer->getOutputFormat();
    if (!m_wavWriter->open(path, fmt)) {
        return false;
    }
    
    // Engine should already be started by UI or Replay, but if not, 
    // we need to know the mode. This implies MainWindow should ensure 
    // engine is running or start it. 
    // For safety, we expect engine to be started by the UI layer which knows the mode.
    
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
    
    emit recordingStopped(path);
}

void RecordingManager::setReplayEnabled(bool enabled, const QString &mode)
{
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
