#include "managers/settingsmanager.h"
#include "storage/StoragePaths.h"
#include <QDir>
#include <QFileInfo>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_replayEnabled(false)
    , m_replayDuration(30)
    , m_baseDirectory(StoragePaths::defaultBaseDirectory())
    , m_hotkeysEnabled(true)
{
}

void SettingsManager::load()
{
    QFile file(getSettingsFilePath());
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    bool sourcesDirty = false;
    bool slotsDirty = false;
    bool replayEnabledDirty = false;
    bool replayDurationDirty = false;
    bool baseDirDirty = false;
    bool recOverrideDirty = false;
    bool replayOverrideDirty = false;
    bool micOutDirty = false;
    bool localMonDirty = false;
    bool micOutDevDirty = false;
    bool localMonDevDirty = false;
    bool micPassDirty = false;
    bool voiceDevDirty = false;
    bool sampleRateDirty = false;
    bool hotkeysEnabledDirty = false;
    bool defaultPlaybackModeDirty = false;

    if (doc.isArray()) {
        QList<AudioSource> newSources;
        QJsonArray arr = doc.array();
        for (const QJsonValue& val : std::as_const(arr)) {
            newSources.append(AudioSource::fromJson(val.toObject()));
        }
        if (newSources != m_sources) {
            m_sources = newSources;
            sourcesDirty = true;
        }
    } else if (doc.isObject()) {
        QJsonObject obj = doc.object();

        QList<AudioSource> newSources;
        QJsonArray arr = obj["sources"].toArray();
        for (const QJsonValue& val : std::as_const(arr)) {
            newSources.append(AudioSource::fromJson(val.toObject()));
        }
        if (newSources != m_sources) {
            m_sources = newSources;
            sourcesDirty = true;
        }

        bool newReplayEnabled = obj["replayEnabled"].toBool(false);
        if (newReplayEnabled != m_replayEnabled) {
            m_replayEnabled = newReplayEnabled;
            replayEnabledDirty = true;
        }

        int newReplayDuration = obj["replayDuration"].toInt(30);
        if (newReplayDuration != m_replayDuration) {
            m_replayDuration = newReplayDuration;
            replayDurationDirty = true;
        }

        QString newBaseDir = obj["baseDirectory"].toString(m_baseDirectory);
        if (newBaseDir != m_baseDirectory) {
            m_baseDirectory = newBaseDir;
            baseDirDirty = true;
        }

        QString newRecOverride = obj["recordingDirectoryOverride"].toString(m_recordingDirectoryOverride);
        if (newRecOverride != m_recordingDirectoryOverride) {
            m_recordingDirectoryOverride = newRecOverride;
            recOverrideDirty = true;
        }

        QString newReplayOverride = obj["replayDirectoryOverride"].toString(m_replayDirectoryOverride);
        if (newReplayOverride != m_replayDirectoryOverride) {
            m_replayDirectoryOverride = newReplayOverride;
            replayOverrideDirty = true;
        }

        bool newMicOutput = obj["enableMicOutput"].toBool(true);
        if (newMicOutput != m_enableMicOutput) {
            m_enableMicOutput = newMicOutput;
            micOutDirty = true;
        }

        bool newLocalMon = obj["enableLocalMonitoring"].toBool(true);
        if (newLocalMon != m_enableLocalMonitoring) {
            m_enableLocalMonitoring = newLocalMon;
            localMonDirty = true;
        }

        QString newMicOutDev = obj["micOutputDevice"].toString("");
        if (newMicOutDev != m_micOutputDevice) {
            m_micOutputDevice = newMicOutDev;
            micOutDevDirty = true;
        }

        QString newLocalMonDev = obj["localMonitorDevice"].toString("");
        if (newLocalMonDev != m_localMonitorDevice) {
            m_localMonitorDevice = newLocalMonDev;
            localMonDevDirty = true;
        }

        bool newMicPass = obj["enableMicPassthrough"].toBool(false);
        if (newMicPass != m_enableMicPassthrough) {
            m_enableMicPassthrough = newMicPass;
            micPassDirty = true;
        }

        QString newVoiceDev = obj["voiceInputDevice"].toString("");
        if (newVoiceDev != m_voiceInputDevice) {
            m_voiceInputDevice = newVoiceDev;
            voiceDevDirty = true;
        }

        int newSampleRate = obj["recordingSampleRate"].toInt(48000);
        if (newSampleRate != m_recordingSampleRate) {
            m_recordingSampleRate = newSampleRate;
            sampleRateDirty = true;
        }

        bool newHotkeysEnabled = obj["hotkeysEnabled"].toBool(true);
        if (newHotkeysEnabled != m_hotkeysEnabled) {
            m_hotkeysEnabled = newHotkeysEnabled;
            hotkeysEnabledDirty = true;
        }

        PlaybackMode newDefaultMode = stringToPlaybackMode(obj["defaultPlaybackMode"].toString("RestartRetrigger"));
        if (newDefaultMode == PlaybackMode::Default) newDefaultMode = PlaybackMode::RestartRetrigger;
        if (newDefaultMode != m_defaultPlaybackMode) {
            m_defaultPlaybackMode = newDefaultMode;
            defaultPlaybackModeDirty = true;
        }

        QList<SoundPlayerSlot> newSlots;
        QJsonArray slotsArr = obj["soundBoardSlots"].toArray();
        for (const QJsonValue& val : std::as_const(slotsArr)) {
            newSlots.append(SoundPlayerSlot::fromJson(val.toObject()));
        }
        if (newSlots != m_soundBoardSlots) {
            m_soundBoardSlots = newSlots;
            slotsDirty = true;
        }
    }

    // Ensure resolved paths exist (all fields assigned by now)
    StoragePaths::ensureDirectoryExists(recordingDirectory());
    StoragePaths::ensureDirectoryExists(replayDirectory());

    if (sourcesDirty)          emit sourcesChanged();
    if (slotsDirty)            emit soundBoardSlotsChanged();
    if (replayEnabledDirty)    emit replayEnabledChanged();
    if (replayDurationDirty)   emit replayDurationChanged();
    if (baseDirDirty)          emit baseDirectoryChanged();
    if (recOverrideDirty)      emit recordingDirectoryOverrideChanged();
    if (replayOverrideDirty)   emit replayDirectoryOverrideChanged();
    if (baseDirDirty || recOverrideDirty)    emit recordingDirectoryChanged();
    if (baseDirDirty || replayOverrideDirty) emit replayDirectoryChanged();
    if (micOutDirty)           emit enableMicOutputChanged();
    if (localMonDirty)         emit enableLocalMonitoringChanged();
    if (micOutDevDirty)        emit micOutputDeviceChanged();
    if (localMonDevDirty)      emit localMonitorDeviceChanged();
    if (micPassDirty)          emit enableMicPassthroughChanged();
    if (voiceDevDirty)         emit voiceInputDeviceChanged();
    if (sampleRateDirty)       emit recordingSampleRateChanged();
    if (hotkeysEnabledDirty)   emit hotkeysEnabledChanged();
    if (defaultPlaybackModeDirty) emit defaultPlaybackModeChanged();
}

void SettingsManager::save()
{
    QJsonObject root;

    QJsonArray sourcesArr;
    for (const AudioSource& src : std::as_const(m_sources)) {
        sourcesArr.append(src.toJson());
    }
    root["sources"] = sourcesArr;

    QJsonArray slotsArr;
    for (const SoundPlayerSlot& slot : std::as_const(m_soundBoardSlots)) {
        slotsArr.append(slot.toJson());
    }
    root["soundBoardSlots"] = slotsArr;

    root["replayEnabled"] = m_replayEnabled;
    root["replayDuration"] = m_replayDuration;
    root["baseDirectory"] = m_baseDirectory;
    root["recordingDirectoryOverride"] = m_recordingDirectoryOverride;
    root["replayDirectoryOverride"] = m_replayDirectoryOverride;
    root["enableMicOutput"] = m_enableMicOutput;
    root["enableLocalMonitoring"] = m_enableLocalMonitoring;
    root["micOutputDevice"] = m_micOutputDevice;
    root["localMonitorDevice"] = m_localMonitorDevice;
    root["enableMicPassthrough"] = m_enableMicPassthrough;
    root["voiceInputDevice"] = m_voiceInputDevice;
    root["recordingSampleRate"] = m_recordingSampleRate;
    root["hotkeysEnabled"] = m_hotkeysEnabled;
    root["defaultPlaybackMode"] = playbackModeToString(m_defaultPlaybackMode);

    QJsonDocument doc(root);
    QFile file(getSettingsFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }
}

QString SettingsManager::getSettingsFilePath() const
{
    return StoragePaths::settingsFilePath();
}

// --- Setters with NOTIFY signals ---

void SettingsManager::setSources(const QList<AudioSource> &sources)
{
    if (m_sources != sources) {
        m_sources = sources;
        emit sourcesChanged();
    }
}

void SettingsManager::setSoundBoardSlots(const QList<SoundPlayerSlot> &soundBoardSlots)
{
    if (m_soundBoardSlots != soundBoardSlots) {
        m_soundBoardSlots = soundBoardSlots;
        emit soundBoardSlotsChanged();
    }
}

void SettingsManager::setReplayEnabled(bool enabled)
{
    if (m_replayEnabled != enabled) {
        m_replayEnabled = enabled;
        emit replayEnabledChanged();
    }
}

void SettingsManager::setReplayDuration(int duration)
{
    if (m_replayDuration != duration) {
        m_replayDuration = duration;
        emit replayDurationChanged();
    }
}

QString SettingsManager::recordingDirectory() const
{
    return m_recordingDirectoryOverride.isEmpty()
        ? QDir::toNativeSeparators(m_baseDirectory + "/recordings")
        : m_recordingDirectoryOverride;
}

QString SettingsManager::replayDirectory() const
{
    return m_replayDirectoryOverride.isEmpty()
        ? QDir::toNativeSeparators(m_baseDirectory + "/replays")
        : m_replayDirectoryOverride;
}

void SettingsManager::setBaseDirectory(const QString &dir)
{
    if (m_baseDirectory == dir) return;
    m_baseDirectory = dir;
    StoragePaths::ensureDirectoryExists(dir);
    emit baseDirectoryChanged();
    if (m_recordingDirectoryOverride.isEmpty()) {
        StoragePaths::ensureDirectoryExists(recordingDirectory());
        emit recordingDirectoryChanged();
    }
    if (m_replayDirectoryOverride.isEmpty()) {
        StoragePaths::ensureDirectoryExists(replayDirectory());
        emit replayDirectoryChanged();
    }
}

void SettingsManager::setRecordingDirectoryOverride(const QString &dir)
{
    if (m_recordingDirectoryOverride == dir) return;
    m_recordingDirectoryOverride = dir;
    StoragePaths::ensureDirectoryExists(recordingDirectory());
    emit recordingDirectoryOverrideChanged();
    emit recordingDirectoryChanged();
}

void SettingsManager::setReplayDirectoryOverride(const QString &dir)
{
    if (m_replayDirectoryOverride == dir) return;
    m_replayDirectoryOverride = dir;
    StoragePaths::ensureDirectoryExists(replayDirectory());
    emit replayDirectoryOverrideChanged();
    emit replayDirectoryChanged();
}

void SettingsManager::setEnableMicOutput(bool enabled)
{
    if (m_enableMicOutput != enabled) {
        m_enableMicOutput = enabled;
        emit enableMicOutputChanged();
    }
}

void SettingsManager::setEnableLocalMonitoring(bool enabled)
{
    if (m_enableLocalMonitoring != enabled) {
        m_enableLocalMonitoring = enabled;
        emit enableLocalMonitoringChanged();
    }
}

void SettingsManager::setMicOutputDevice(const QString &device)
{
    if (m_micOutputDevice != device) {
        m_micOutputDevice = device;
        emit micOutputDeviceChanged();
    }
}

void SettingsManager::setLocalMonitorDevice(const QString &device)
{
    if (m_localMonitorDevice != device) {
        m_localMonitorDevice = device;
        emit localMonitorDeviceChanged();
    }
}

void SettingsManager::setEnableMicPassthrough(bool enabled)
{
    if (m_enableMicPassthrough != enabled) {
        m_enableMicPassthrough = enabled;
        emit enableMicPassthroughChanged();
    }
}

void SettingsManager::setVoiceInputDevice(const QString &device)
{
    if (m_voiceInputDevice != device) {
        m_voiceInputDevice = device;
        emit voiceInputDeviceChanged();
    }
}

void SettingsManager::setRecordingSampleRate(int sampleRate)
{
    if (m_recordingSampleRate != sampleRate) {
        m_recordingSampleRate = sampleRate;
        emit recordingSampleRateChanged();
    }
}

void SettingsManager::setHotkeysEnabled(bool enabled)
{
    if (m_hotkeysEnabled != enabled) {
        m_hotkeysEnabled = enabled;
        emit hotkeysEnabledChanged();
    }
}

void SettingsManager::setDefaultPlaybackMode(PlaybackMode mode)
{
    if (mode == PlaybackMode::Default) {
        mode = PlaybackMode::RestartRetrigger;
    }
    if (m_defaultPlaybackMode != mode) {
        m_defaultPlaybackMode = mode;
        emit defaultPlaybackModeChanged();
    }
}
