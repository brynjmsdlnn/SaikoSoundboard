#ifndef SOUNDPLAYERSLOTMODEL_H
#define SOUNDPLAYERSLOTMODEL_H

#include <QAbstractListModel>
#include <QDir>
#include "models/soundplayerslot.h"
#include "managers/soundboardmanager.h"

class SoundPlayerSlotModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        SlotIdRole = Qt::UserRole + 1,
        NameRole,
        FilePathRole,
        PlayHotkeyRole,
        AssignHotkeyRole,
        VolumeRole,
        OutputRoutingRole,
        StartTimeMsRole,
        EndTimeMsRole,
        IsTemporaryRole,
        DurationSecRole,
        LockedRole,
        FileExistsRole
    };

    explicit SoundPlayerSlotModel(SoundboardManager *manager, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setVolume(int row, float volume);
    Q_INVOKABLE void setRouting(int row, int routing);
    Q_INVOKABLE void setClipRange(int row, qint64 startMs, qint64 endMs);
    Q_INVOKABLE QVariantMap get(int row) const;

signals:
    void slotRemoved(const QString &id);

private slots:
    void onSlotsChanged();

private:
    SoundboardManager *m_manager;
    QList<SoundPlayerSlot> m_slots;
    bool m_updating = false;

    int rowForId(const QString &id) const;
};

#endif
