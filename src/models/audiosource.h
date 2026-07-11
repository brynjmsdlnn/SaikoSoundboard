#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include <QString>
#include <QJsonObject>
#include <QUuid>
#include <QMetaType>

struct AudioSource {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id CONSTANT)
    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(QString executableName MEMBER executableName CONSTANT)
    Q_PROPERTY(QString type MEMBER type CONSTANT)
    Q_PROPERTY(QString deviceName MEMBER deviceName CONSTANT)
    Q_PROPERTY(bool monitor MEMBER monitor CONSTANT)

public:
    QString id;
    QString name;
    QString executableName;
    QString executablePath;
    bool enabled = true;
    float volume = 1.0f;
    bool solo = false;
    QString type = "process"; // "process" or "device"
    QString deviceName;
    bool monitor = true;

    AudioSource() {
        id = QUuid::createUuid().toString();
    }

    bool operator==(const AudioSource &other) const {
        return id == other.id && name == other.name && executableName == other.executableName
            && executablePath == other.executablePath && enabled == other.enabled
            && qFuzzyCompare(volume, other.volume) && solo == other.solo
            && type == other.type && deviceName == other.deviceName && monitor == other.monitor;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["name"] = name;
        obj["executableName"] = executableName;
        obj["executablePath"] = executablePath;
        obj["enabled"] = enabled;
        obj["volume"] = volume;
        obj["solo"] = solo;
        obj["type"] = type;
        obj["deviceName"] = deviceName;
        obj["monitor"] = monitor;
        return obj;
    }

    static AudioSource fromJson(const QJsonObject& obj) {
        AudioSource src;
        src.id = obj["id"].toString(QUuid::createUuid().toString());
        src.name = obj["name"].toString();
        src.executableName = obj["executableName"].toString();
        src.executablePath = obj["executablePath"].toString();
        src.enabled = obj["enabled"].toBool(true);
        src.volume = obj["volume"].toDouble(1.0);
        src.solo = obj["solo"].toBool(false);
        src.type = obj["type"].toString("process");
        src.deviceName = obj["deviceName"].toString();
        src.monitor = obj["monitor"].toBool(true);
        return src;
    }
};

Q_DECLARE_METATYPE(AudioSource)

#endif // AUDIOSOURCE_H
