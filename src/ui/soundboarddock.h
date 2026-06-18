#ifndef SOUNDBOARDDOCK_H
#define SOUNDBOARDDOCK_H

#include <QDockWidget>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include "managers/soundboardmanager.h"

#include <QLabel>
#include <QMouseEvent>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(const QString &text, QWidget *parent = nullptr) : QLabel(text, parent) {
        setCursor(Qt::PointingHandCursor);
    }
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            emit clicked();
        }
        QLabel::mousePressEvent(event);
    }
};

class QHBoxLayout;
class QScrollArea;
class ActionManager;
class WaveformWidget;
class QmlBackend;

class SoundboardDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit SoundboardDock(SoundboardManager *manager, ActionManager *actionManager, QmlBackend *qmlBackend, QWidget *parent = nullptr);

public slots:
    void refresh();

private slots:
    void onAddPlayer();
    void onRemovePlayer(const QString &id);
    void onAssignFile(const QString &id);
    void onAssignReplay(const QString &id, bool preserveExisting);
    void onPlayPlayer(const QString &id);
    void onPlayPreview(const QString &id);
    void onStopPlayer(const QString &id);
    void onVolumeChanged(const QString &id, int volume);
    void onRenamePlayer(const QString &id);
    void onHotkeySetup(const QString &id);
    void onMakePermanent(const QString &id);
    void onWaveformGenerated(const QString &playerId, const WaveformData &data);
    void onPlayerPositionChanged(const QString &playerId, qint64 position);
    void onHotkeyDialogFinished();

private:
    SoundboardManager *m_manager;
    ActionManager *m_actionManager;
    QmlBackend *m_qmlBackend;
    QWidget *m_scrollContent;
    QHBoxLayout *m_scrollLayout;
    QScrollArea *m_scrollArea;
    QMap<QString, WaveformWidget*> m_waveformWidgets;
    QPointer<QQuickWindow> m_hotkeyDialogWindow;

    void clearLayout();
};

#endif // SOUNDBOARDDOCK_H
