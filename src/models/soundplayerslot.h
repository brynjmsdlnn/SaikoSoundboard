#ifndef SOUNDPLAYERSLOT_H
#define SOUNDPLAYERSLOT_H

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

struct SoundPlayerSlot {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id CONSTANT)
    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(QString filePath MEMBER filePath CONSTANT)
    Q_PROPERTY(QString playHotkey MEMBER playHotkey CONSTANT)
    Q_PROPERTY(QString assignHotkey MEMBER assignHotkey CONSTANT)
    Q_PROPERTY(float volume MEMBER volume CONSTANT)
    Q_PROPERTY(bool enabled MEMBER enabled CONSTANT)
    Q_PROPERTY(OutputRouting outputRouting MEMBER outputRouting CONSTANT)
    Q_PROPERTY(qint64 startTimeMs MEMBER startTimeMs CONSTANT)
    Q_PROPERTY(qint64 endTimeMs MEMBER endTimeMs CONSTANT)

public:
    QString id;
    QString name;
    QString filePath;
    QString playHotkey;
    QString assignHotkey;
    float volume = 1.0f;
    bool enabled = true;
    OutputRouting outputRouting = OutputRouting::Both;
    qint64 startTimeMs = 0;
    qint64 endTimeMs = -1;

    SoundPlayerSlot() {
        id = QUuid::createUuid().toString();
    }

    bool operator==(const SoundPlayerSlot &other) const {
        return id == other.id && name == other.name && filePath == other.filePath
            && playHotkey == other.playHotkey && assignHotkey == other.assignHotkey
            && qFuzzyCompare(volume, other.volume) && enabled == other.enabled
            && outputRouting == other.outputRouting && startTimeMs == other.startTimeMs
            && endTimeMs == other.endTimeMs;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["name"] = name;
        obj["filePath"] = filePath;
        obj["playHotkey"] = playHotkey;
        obj["assignHotkey"] = assignHotkey;
        obj["volume"] = static_cast<double>(volume);
        obj["enabled"] = enabled;
        obj["outputRouting"] = outputRoutingToString(outputRouting);
        obj["startTimeMs"] = startTimeMs;
        obj["endTimeMs"] = endTimeMs;
        return obj;
    }

    static SoundPlayerSlot fromJson(const QJsonObject& obj) {
        SoundPlayerSlot slot;
        slot.id = obj["id"].toString(QUuid::createUuid().toString());
        slot.name = obj["name"].toString();
        slot.filePath = obj["filePath"].toString();
        slot.playHotkey = obj["playHotkey"].toString();
        slot.assignHotkey = obj["assignHotkey"].toString();
        slot.volume = static_cast<float>(obj["volume"].toDouble(1.0));
        slot.enabled = obj["enabled"].toBool(true);
        slot.outputRouting = stringToOutputRouting(obj["outputRouting"].toString("Both"));
        slot.startTimeMs = obj["startTimeMs"].toVariant().toLongLong();
        slot.endTimeMs = obj["endTimeMs"].toVariant().toLongLong();
        if (obj.contains("endTimeMs") == false) {
            slot.endTimeMs = -1;
        }
        return slot;
    }
};

Q_DECLARE_METATYPE(SoundPlayerSlot)

#endif // SOUNDPLAYERSLOT_H
