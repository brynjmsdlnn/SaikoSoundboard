#ifndef SOUNDPLAYERSLOT_H
#define SOUNDPLAYERSLOT_H

#include <QString>
#include <QJsonObject>
#include <QUuid>

struct SoundPlayerSlot {
    QString id;
    QString name;
    QString filePath;
    QString playHotkey;
    QString assignHotkey;
    float volume = 1.0f;
    bool enabled = true;

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
        return slot;
    }
};

#endif // SOUNDPLAYERSLOT_H
