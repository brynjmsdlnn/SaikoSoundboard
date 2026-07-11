#ifndef AUDIOSOURCELISTMODEL_H
#define AUDIOSOURCELISTMODEL_H

#include <QAbstractListModel>
#include "models/audiosource.h"

class SettingsManager;

class AudioSourceListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        SourceIdRole = Qt::UserRole + 1,
        NameRole,
        ExecutableNameRole,
        ExecutablePathRole,
        EnabledRole,
        VolumeRole,
        SoloRole,
        TypeRole,
        DeviceNameRole,
        MonitorRole
    };

    explicit AudioSourceListModel(SettingsManager *settings, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool addSource(const QString &name, const QString &executableName, const QString &executablePath);
    Q_INVOKABLE bool addDeviceSource(const QString &name, const QString &deviceName);
    Q_INVOKABLE void removeSource(const QString &sourceId);
    Q_INVOKABLE QString getSourceId(int row) const;
    Q_INVOKABLE bool hasExecutable(const QString &executableName) const;
    Q_INVOKABLE bool hasDevice(const QString &deviceName) const;
    Q_INVOKABLE bool setSolo(const QString &sourceId, bool solo);
    Q_INVOKABLE bool setEnabled(const QString &sourceId, bool enabled);
    Q_INVOKABLE bool setVolume(const QString &sourceId, float volume);
    Q_INVOKABLE bool setMonitor(const QString &sourceId, bool monitor);

private slots:
    void onSourcesChanged();

private:
    SettingsManager *m_settings;
    QList<AudioSource> m_sources;
};

#endif
