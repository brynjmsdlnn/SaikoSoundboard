#include "managers/settingsmanager.h"
#include <QDir>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_replayEnabled(false)
    , m_replayDuration(30)
{
    m_saveDirectory = QDir::homePath() + "/Recordings/Saiko Soundboard";
}

void SettingsManager::load()
{
    QFile file(getSettingsFilePath());
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isArray()) {
        m_sources.clear();
        QJsonArray arr = doc.array();
        for (const QJsonValue& val : std::as_const(arr)) {
            m_sources.append(AudioSource::fromJson(val.toObject()));
        }
    } else if (doc.isObject()) {
        QJsonObject obj = doc.object();
        
        m_sources.clear();
        QJsonArray arr = obj["sources"].toArray();
        for (const QJsonValue& val : std::as_const(arr)) {
            m_sources.append(AudioSource::fromJson(val.toObject()));
        }
        
        m_replayEnabled = obj["replayEnabled"].toBool(false);
        m_replayDuration = obj["replayDuration"].toInt(30);
        m_saveDirectory = obj["saveDirectory"].toString(m_saveDirectory);
        m_enableMicOutput = obj["enableMicOutput"].toBool(true);
        m_enableLocalMonitoring = obj["enableLocalMonitoring"].toBool(true);
        m_micOutputDevice = obj["micOutputDevice"].toString("");
        m_localMonitorDevice = obj["localMonitorDevice"].toString("");
        m_enableMicPassthrough = obj["enableMicPassthrough"].toBool(false);
        m_voiceInputDevice = obj["voiceInputDevice"].toString("");

        m_soundBoardSlots.clear();
        QJsonArray slotsArr = obj["soundBoardSlots"].toArray();
        for (const QJsonValue& val : std::as_const(slotsArr)) {
            m_soundBoardSlots.append(SoundPlayerSlot::fromJson(val.toObject()));
        }
    }
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
