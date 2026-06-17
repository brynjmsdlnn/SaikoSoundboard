#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include <QString>
#include <QJsonObject>
#include <QUuid>

struct AudioSource {
    QString id;
    QString name;
    QString executableName;
    QString executablePath;
    bool enabled = true;
    float volume = 1.0f;

    AudioSource() {
        id = QUuid::createUuid().toString();
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

#endif // AUDIOSOURCE_H
