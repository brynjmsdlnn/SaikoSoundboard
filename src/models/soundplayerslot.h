#ifndef SOUNDPLAYERSLOT_H
#define SOUNDPLAYERSLOT_H

#include "logging/LogMacros.h"
#include <QString>
#include <QJsonObject>
#include <QUuid>
#include <QMetaType>
namespace SaikoOutput {

Q_NAMESPACE

enum OutputRouting {
    Both = 0,
    MicOnly = 1,
    LocalOnly = 2
};
Q_ENUM_NS(OutputRouting)

} // namespace SaikoOutput

using SaikoOutput::OutputRouting;

namespace SaikoPlayback {

Q_NAMESPACE

enum PlayState {
    Stopped = 0,
    Playing = 1,
    Preview = 2
};
Q_ENUM_NS(PlayState)

enum PlaybackMode {
    Default          = 0,
    RestartRetrigger = 1,
    ToggleStop       = 2,
    QueuedSequential = 3,
    LayeredCutAll    = 4,
    LayeredRingOut   = 5
};
Q_ENUM_NS(PlaybackMode)

} // namespace SaikoPlayback

using SaikoPlayback::PlayState;
using SaikoPlayback::PlaybackMode;

inline QString outputRoutingToString(OutputRouting routing) {
    switch (routing) {
        case OutputRouting::MicOnly: return "MicOnly";
        case OutputRouting::LocalOnly: return "LocalOnly";
        case OutputRouting::Both:
        default: return "Both";
    }
}

inline OutputRouting stringToOutputRouting(const QString& str) {
    if (str == "MicOnly") return OutputRouting::MicOnly;
    if (str == "LocalOnly") return OutputRouting::LocalOnly;
    return OutputRouting::Both;
}

inline QString playbackModeToString(PlaybackMode mode) {
    switch (mode) {
        case PlaybackMode::RestartRetrigger: return "RestartRetrigger";
        case PlaybackMode::ToggleStop: return "ToggleStop";
        case PlaybackMode::QueuedSequential: return "QueuedSequential";
        case PlaybackMode::LayeredCutAll: return "LayeredCutAll";
        case PlaybackMode::LayeredRingOut: return "LayeredRingOut";
        case PlaybackMode::Default:
        default: return "Default";
    }
}

inline PlaybackMode stringToPlaybackMode(const QString& str) {
    if (str == "RestartRetrigger") return PlaybackMode::RestartRetrigger;
    if (str == "ToggleStop") return PlaybackMode::ToggleStop;
    if (str == "QueuedSequential") return PlaybackMode::QueuedSequential;
    if (str == "LayeredCutAll") return PlaybackMode::LayeredCutAll;
    if (str == "LayeredRingOut") return PlaybackMode::LayeredRingOut;
    if (str == "Default") return PlaybackMode::Default;
    LOG_WARN(LogCategory::Settings,
             QStringLiteral("[Settings] Unknown PlaybackMode string (string: \"%1\") \u2014 falling back to Default").arg(str));
    return PlaybackMode::Default;
}

struct SoundPlayerSlot {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id CONSTANT)
    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(QString filePath MEMBER filePath CONSTANT)
    Q_PROPERTY(QString playHotkey MEMBER playHotkey CONSTANT)
    Q_PROPERTY(QString assignHotkey MEMBER assignHotkey CONSTANT)
    Q_PROPERTY(float volume MEMBER volume CONSTANT)
    Q_PROPERTY(OutputRouting outputRouting MEMBER outputRouting CONSTANT)
    Q_PROPERTY(PlaybackMode playbackMode MEMBER playbackMode CONSTANT)
    Q_PROPERTY(qint64 startTimeMs MEMBER startTimeMs CONSTANT)
    Q_PROPERTY(qint64 endTimeMs MEMBER endTimeMs CONSTANT)
    Q_PROPERTY(bool locked MEMBER locked CONSTANT)

public:
    QString id;
    QString name;
    QString filePath;
    QString playHotkey;
    QString assignHotkey;
    float volume = 1.0f;
    bool locked = false;
    OutputRouting outputRouting = OutputRouting::Both;
    PlaybackMode playbackMode = PlaybackMode::Default;
    qint64 startTimeMs = 0;
    qint64 endTimeMs = -1;

    SoundPlayerSlot() {
        id = QUuid::createUuid().toString();
    }

    bool operator==(const SoundPlayerSlot &other) const {
        return id == other.id && name == other.name && filePath == other.filePath
            && playHotkey == other.playHotkey && assignHotkey == other.assignHotkey
            && qFuzzyCompare(volume, other.volume) && locked == other.locked
            && outputRouting == other.outputRouting && playbackMode == other.playbackMode
            && startTimeMs == other.startTimeMs && endTimeMs == other.endTimeMs;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["name"] = name;
        obj["filePath"] = filePath;
        obj["playHotkey"] = playHotkey;
        obj["assignHotkey"] = assignHotkey;
        obj["volume"] = static_cast<double>(volume);
        obj["outputRouting"] = outputRoutingToString(outputRouting);
        obj["playbackMode"] = playbackModeToString(playbackMode);
        obj["startTimeMs"] = startTimeMs;
        obj["endTimeMs"] = endTimeMs;
        obj["locked"] = locked;
        return obj;
    }

    static SoundPlayerSlot fromJson(const QJsonObject& obj) {
        SoundPlayerSlot slot;
        slot.id = obj["id"].toString(QUuid::createUuid().toString());
        slot.name = obj["name"].toString();
        slot.filePath = obj["filePath"].toString();
        slot.playHotkey = obj["playHotkey"].toString();
        slot.assignHotkey = obj["assignHotkey"].toString();
        slot.volume = static_float_cast_or_toDouble(obj);
        slot.outputRouting = stringToOutputRouting(obj["outputRouting"].toString("Both"));
        slot.playbackMode = stringToPlaybackMode(obj["playbackMode"].toString("Default"));
        slot.startTimeMs = obj["startTimeMs"].toVariant().toLongLong();
        slot.endTimeMs = obj["endTimeMs"].toVariant().toLongLong();
        if (obj.contains("endTimeMs") == false) {
            slot.endTimeMs = -1;
        }
        slot.locked = obj["locked"].toBool(false);
        return slot;
    }

private:
    static float static_float_cast_or_toDouble(const QJsonObject& obj) {
        return static_cast<float>(obj["volume"].toDouble(1.0));
    }
};

Q_DECLARE_METATYPE(SoundPlayerSlot)

#endif // SOUNDPLAYERSLOT_H
