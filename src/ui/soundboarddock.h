#ifndef SOUNDBOARDDOCK_H
#define SOUNDBOARDDOCK_H

#include <QDockWidget>
#include <QList>
#include <QMap>
#include "managers/soundboardmanager.h"

class QHBoxLayout;
class QScrollArea;
class ActionManager;

class SoundboardDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit SoundboardDock(SoundboardManager *manager, ActionManager *actionManager, QWidget *parent = nullptr);

public slots:
    void refresh();

private slots:
    void onAddPlayer();
    void onRemovePlayer(const QString &id);
    void onAssignFile(const QString &id);
    void onAssignReplay(const QString &id, bool preserveExisting);
    void onPlayPlayer(const QString &id);
    void onStopPlayer(const QString &id);
    void onVolumeChanged(const QString &id, int volume);
    void onRenamePlayer(const QString &id);
    void onHotkeySetup(const QString &id);
    void onMakePermanent(const QString &id);

private:
    SoundboardManager *m_manager;
    ActionManager *m_actionManager;
    QWidget *m_scrollContent;
    QHBoxLayout *m_scrollLayout;
    QScrollArea *m_scrollArea;

    void clearLayout();
};

#endif // SOUNDBOARDDOCK_H
