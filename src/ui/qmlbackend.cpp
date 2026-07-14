#include "ui/qmlbackend.h"
#include "ui/realtimewaveformitem.h"
#include "ui/waveformitem.h"
#include "audio/wasapirecorder.h"
#include "storage/StoragePaths.h"
#include <QDateTime>
#include <QFileInfo>
#include <QUrl>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include "core/adapters/WindowsHotkeyBackend.h"
#include "core/adapters/WindowsProcessFinder.h"
#include "managers/settingsmanager.h"
#include "managers/recordingmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/actionmanager.h"
#include "managers/hotkeymanager.h"
#include "models/soundplayerslotmodel.h"
#include "models/audiosourcelistmodel.h"

QmlBackend::QmlBackend(QObject *parent)
    : QObject(parent)
{
    m_settings = new SettingsManager(this);
    m_settings->load();

    m_recordingManager = new RecordingManager(m_settings, this);

    m_soundboardManager = new SoundboardManager(m_settings, this);

    m_actionManager = new ActionManager(m_soundboardManager, m_recordingManager, m_settings, this);

    auto *backend = new Saiko::Adapters::WindowsHotkeyBackend();
    m_hotkeyBackend = backend;
    m_hotkeyManager = new HotkeyManager(m_actionManager, backend, this);

    m_slotModel = new SoundPlayerSlotModel(m_soundboardManager, this);
    m_sourceModel = new AudioSourceListModel(m_settings, this);

    auto updateActiveHotkeys = [this]() {
        QMap<QString, Action> hotkeyMap;
        if (m_settings->hotkeysEnabled()) {
            for (const auto &slot : m_soundboardManager->getSlots()) {
                if (!slot.playHotkey.isEmpty())
                    hotkeyMap[slot.playHotkey] = Action::createPlay(slot.id);
                if (!slot.assignHotkey.isEmpty())
                    hotkeyMap[slot.assignHotkey] = Action::createAssignReplay(slot.id);
            }
        }
        m_hotkeyManager->updateHotkeys(hotkeyMap);
    };

    connect(m_soundboardManager, &SoundboardManager::slotsChanged, this, updateActiveHotkeys);
    connect(m_settings, &SettingsManager::hotkeysEnabledChanged, this, updateActiveHotkeys);

    m_soundboardManager->loadFromSettings();

    connect(m_recordingManager, &RecordingManager::stateChanged, this, &QmlBackend::captureStateChanged);
    connect(m_recordingManager, &RecordingManager::stateChanged, this, [this](CaptureState state) {
        if (state == CaptureState::Idle) {
            m_replayWaveformTimer->stop();
            updateReplayWaveform();
        } else {
            if (!m_replayWaveformTimer->isActive())
                m_replayWaveformTimer->start();
        }
    });

    m_replayWaveformTimer = new QTimer(this);
    m_replayWaveformTimer->setInterval(200);
    connect(m_replayWaveformTimer, &QTimer::timeout, this, &QmlBackend::updateReplayWaveform);

    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
        m_isPlaying = (state == QMediaPlayer::PlayingState);
        emit playbackStateChanged();
    });
    connect(m_player, &QMediaPlayer::durationChanged, this, [this](qint64 dur) {
        m_playbackDuration = dur;
        emit playbackDurationChanged();
    });
    connect(m_player, &QMediaPlayer::positionChanged, this, [this](qint64) {
        emit playbackPositionChanged();
    });
    connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &) {
        m_isPlaying = false;
        emit playbackStateChanged();
    });

    // Wire up recording PCM capture and state signals
    connect(m_recordingManager->mixer(), &AudioMixer::mixedPcmReady, this, [this](const QByteArray &data) {
        if (m_recordingManager->isRecording()) {
            m_recordingPcm.append(data);
        }
    });
    connect(m_recordingManager, &RecordingManager::recordingStarted, this, [this](const QString &) {
        m_recordingPcm.clear();
        m_recordingWaveform = WaveformData();
        emit recordingWaveformChanged();
    });
    connect(m_recordingManager, &RecordingManager::recordingStopped, this, [this](const QString &path) {
        loadRecordingWaveform(path);
    });

}

QmlBackend::~QmlBackend()
{
    m_soundboardManager->saveToSettings();
    m_settings->save();
    delete static_cast<Saiko::Adapters::WindowsHotkeyBackend*>(m_hotkeyBackend);
}

void QmlBackend::updateReplayWaveform()
{
    if (m_recordingManager && m_recordingManager->replayBuffer()) {
        QByteArray rawPcm = m_recordingManager->replayBuffer()->getBufferData();
        WAVEFORMATEXTENSIBLE fmt = m_recordingManager->mixer()->getOutputFormat();
        if (!rawPcm.isEmpty() && fmt.Format.nSamplesPerSec > 0) {
            m_replayWaveform = WaveformGenerator::generateFromPcm(rawPcm, fmt, 256);
            emit replayWaveformChanged();
        }
    }

    if (m_recordingManager && m_recordingManager->isRecording()) {
        WAVEFORMATEXTENSIBLE fmt = m_recordingManager->mixer()->getOutputFormat();
        if (!m_recordingPcm.isEmpty() && fmt.Format.nSamplesPerSec > 0) {
            m_recordingWaveform = WaveformGenerator::generateFromPcm(m_recordingPcm, fmt, 256);
            emit recordingWaveformChanged();
        }
    }
}

void QmlBackend::playFile(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.exists() || fi.size() <= 100) return;
    m_player->setSource(QUrl::fromLocalFile(path));
    m_player->play();
}

void QmlBackend::stopPlayback()
{
    m_player->stop();
}

QString QmlBackend::renameRecordingFile(const QString &oldPath, const QString &dir, const QString &newName)
{
    QString newPath = dir + "/" + newName;
    if (!newPath.endsWith(".wav", Qt::CaseInsensitive))
        newPath += ".wav";

    QFileInfo check(newPath);
    int counter = 1;
    QString finalPath = newPath;
    while (check.exists()) {
        QString base = QFileInfo(newPath).absolutePath() + "/" + QFileInfo(newPath).completeBaseName();
        finalPath = QString("%1 (%2).wav").arg(base).arg(counter++);
        check = QFileInfo(finalPath);
    }

    if (QFile::rename(oldPath, finalPath))
        return finalPath;
    return oldPath;
}

QVariantList QmlBackend::getRunningProcesses() const
{
    QVariantList result;
    auto processes = Saiko::Adapters::WindowsProcessFinder::getRunningProcesses();
    auto activeSoundExes = Saiko::Adapters::WindowsProcessFinder::getProcessesProducingSound();
    for (const auto &proc : std::as_const(processes)) {
        QVariantMap map;
        map["name"] = proc.first;
        map["fullPath"] = proc.second;
        map["isProducingSound"] = activeSoundExes.contains(proc.first.toLower());
        result.append(map);
    }
    return result;
}

QStringList QmlBackend::getProcessesProducingSound() const
{
    QStringList result;
    auto activeSoundExes = Saiko::Adapters::WindowsProcessFinder::getProcessesProducingSound();
    for (const auto &name : activeSoundExes) {
        result.append(name);
    }
    return result;
}

static bool isVirtualDevice(const QString &description)
{
    QString d = description.toLower();
    return d.contains("cable") || d.contains("virtual") || d.contains("voicemeeter")
        || d.contains("vb-audio") || d.contains("sonar") || d.contains("loopback")
        || d.contains("vac") || d.contains("wave link") || d.contains("soundpad");
}

QVariantList QmlBackend::getAudioOutputDevices() const
{
    QVariantList result;
    const auto devices = QMediaDevices::audioOutputs();
    QString defaultDesc = QMediaDevices::defaultAudioOutput().description();
    for (const auto &dev : devices) {
        QVariantMap map;
        map["description"] = dev.description();
        map["isDefault"] = (dev.description() == defaultDesc);
        map["isVirtual"] = isVirtualDevice(dev.description());
        result.append(map);
    }
    return result;
}

QVariantList QmlBackend::getAudioInputDevices() const
{
    QVariantList result;
    const auto devices = QMediaDevices::audioInputs();
    QString defaultDesc = QMediaDevices::defaultAudioInput().description();
    for (const auto &dev : devices) {
        QVariantMap map;
        map["description"] = dev.description();
        map["isDefault"] = (dev.description() == defaultDesc);
        result.append(map);
    }
    return result;
}

int QmlBackend::systemDefaultSampleRate() const
{
    return WasapiRecorder::systemMixSampleRate();
}

qint64 QmlBackend::recordingFileSize() const
{
    return (m_recordingManager && m_recordingManager->wavWriter())
        ? m_recordingManager->wavWriter()->size() : 0;
}

CaptureState QmlBackend::captureState() const
{
    return m_recordingManager ? m_recordingManager->state() : CaptureState::Idle;
}

void QmlBackend::loadRecordingWaveform(const QString &filePath)
{
    m_recordingPcm.clear();
    m_recordingWaveform = WaveformGenerator::generate(filePath, 256);
    emit recordingWaveformChanged();
}

// --------------------------------------------------------------------------
// Path helpers for QML
// --------------------------------------------------------------------------

QString QmlBackend::generateRecordingFilePath() const
{
    const QString ts = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    return m_settings->recordingDirectory() + QStringLiteral("/Recording_%1.wav").arg(ts);
}

QString QmlBackend::generateReplayFilePath() const
{
    const QString ts = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    return m_settings->replayDirectory() + QStringLiteral("/Replay_%1.wav").arg(ts);
}

QString QmlBackend::defaultRecordingDirectory() const
{
    return StoragePaths::defaultRecordingDirectory();
}

QString QmlBackend::defaultReplayDirectory() const
{
    return StoragePaths::defaultReplayDirectory();
}

QString QmlBackend::defaultBaseDirectory() const
{
    return StoragePaths::defaultBaseDirectory();
}

QString QmlBackend::logDirectory() const
{
    return StoragePaths::logDirectory();
}
