#ifndef SOUNDPLAYERSLOT_H
#define SOUNDPLAYERSLOT_H

#include <QString>
#include <QJsonObject>
#include <QUuid>

enum class OutputRouting {
    Both = 0,
    MicOnly = 1,
    LocalOnly = 2
};

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

#endif // SOUNDPLAYERSLOT_H
