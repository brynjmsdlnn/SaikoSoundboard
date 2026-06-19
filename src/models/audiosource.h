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
    Q_PROPERTY(QString executablePath MEMBER executablePath CONSTANT)
    Q_PROPERTY(bool enabled MEMBER enabled CONSTANT)
    Q_PROPERTY(float volume MEMBER volume CONSTANT)

public:
    QString id;
    QString name;
    QString executableName;
    QString executablePath;
    bool enabled = true;
    float volume = 1.0f;

    AudioSource() {
        id = QUuid::createUuid().toString();
    }

    bool operator==(const AudioSource &other) const {
        return id == other.id && name == other.name && executableName == other.executableName
            && executablePath == other.executablePath && enabled == other.enabled
            && qFuzzyCompare(volume, other.volume);
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["name"] = name;
        obj["executableName"] = executableName;
        obj["executablePath"] = executablePath;
        obj["enabled"] = enabled;
        obj["volume"] = volume;
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
        return src;
    }
};

Q_DECLARE_METATYPE(AudioSource)

#endif // AUDIOSOURCE_H
