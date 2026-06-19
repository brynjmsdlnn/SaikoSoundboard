#include "managers/settingsmanager.h"
#include <QDir>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_replayEnabled(false)
    , m_replayDuration(30)
{
    m_saveDirectory = QDir::toNativeSeparators(QDir::homePath() + "/Saiko Soundboard/recordings");
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
    bool saveDirDirty = false;
    bool micOutDirty = false;
    bool localMonDirty = false;
    bool micOutDevDirty = false;
    bool localMonDevDirty = false;
    bool micPassDirty = false;
    bool voiceDevDirty = false;

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

        QString newSaveDir = obj["saveDirectory"].toString(m_saveDirectory);
        if (newSaveDir != m_saveDirectory) {
            m_saveDirectory = newSaveDir;
            saveDirDirty = true;
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

    if (sourcesDirty)        emit sourcesChanged();
    if (slotsDirty)          emit soundBoardSlotsChanged();
    if (replayEnabledDirty)  emit replayEnabledChanged();
    if (replayDurationDirty) emit replayDurationChanged();
    if (saveDirDirty)        emit saveDirectoryChanged();
    if (micOutDirty)         emit enableMicOutputChanged();
    if (localMonDirty)       emit enableLocalMonitoringChanged();
    if (micOutDevDirty)      emit micOutputDeviceChanged();
    if (localMonDevDirty)    emit localMonitorDeviceChanged();
    if (micPassDirty)        emit enableMicPassthroughChanged();
    if (voiceDevDirty)       emit voiceInputDeviceChanged();
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
    root["saveDirectory"] = m_saveDirectory;
    root["enableMicOutput"] = m_enableMicOutput;
    root["enableLocalMonitoring"] = m_enableLocalMonitoring;
    root["micOutputDevice"] = m_micOutputDevice;
    root["localMonitorDevice"] = m_localMonitorDevice;
    root["enableMicPassthrough"] = m_enableMicPassthrough;
    root["voiceInputDevice"] = m_voiceInputDevice;

    QJsonDocument doc(root);
    QFile file(getSettingsFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }
}

QString SettingsManager::getSettingsFilePath() const
{
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appData);
    return appData + "/settings.json";
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

void SettingsManager::setSaveDirectory(const QString &dir)
{
    if (m_saveDirectory != dir) {
        m_saveDirectory = dir;
        emit saveDirectoryChanged();
    }
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
