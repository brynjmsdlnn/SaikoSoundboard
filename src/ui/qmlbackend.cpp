#include "ui/qmlbackend.h"
#include "ui/realtimewaveformitem.h"
#include "ui/waveformitem.h"
#include "audio/wasapirecorder.h"
#include <QFileInfo>
#include <QUrl>
#include <QMediaDevices>
#include <QAudioDevice>
#include "core/adapters/WindowsHotkeyBackend.h"
#include "core/adapters/WindowsProcessFinder.h"

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

    connect(m_soundboardManager, &SoundboardManager::slotsChanged, this, [this]() {
        QMap<QString, Action> hotkeyMap;
        for (const auto &slot : m_soundboardManager->getSlots()) {
            if (!slot.playHotkey.isEmpty())
                hotkeyMap[slot.playHotkey] = Action::createPlay(slot.id);
            if (!slot.assignHotkey.isEmpty())
                hotkeyMap[slot.assignHotkey] = Action::createAssignReplay(slot.id);
        }
        m_hotkeyManager->updateHotkeys(hotkeyMap);
    });

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
    connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &) {
        m_isPlaying = false;
        emit playbackStateChanged();
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
    if (!m_recordingManager || !m_recordingManager->replayBuffer()) return;
    QByteArray rawPcm = m_recordingManager->replayBuffer()->getBufferData();
    WAVEFORMATEXTENSIBLE fmt = m_recordingManager->mixer()->getOutputFormat();
    if (rawPcm.isEmpty() || fmt.Format.nSamplesPerSec <= 0) return;

    m_replayWaveform = WaveformGenerator::generateFromPcm(rawPcm, fmt, 256);
    emit replayWaveformChanged();
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
    for (const auto &proc : std::as_const(processes)) {
        QVariantMap map;
        map["name"] = proc.first;
        map["fullPath"] = proc.second;
        result.append(map);
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

qint64 QmlBackend::playbackPosition() const
{
    return m_player ? m_player->position() : 0;
}
