#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QJsonArray>
#include <QJsonDocument>
#include <QElapsedTimer>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include "managers/settingsmanager.h"
#include "managers/recordingmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/actionmanager.h"
#include "managers/hotkeymanager.h"
#include "models/audiosource.h"
#include "ui/qmlbackend.h"

class QLabel;
class QPushButton;
class QComboBox;
class QLineEdit;
class QDockWidget;
class QQuickWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onStartRecording();
    void onStopRecording();
    void onUpdateTimer();
    void onPlayLastRecording();
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void onStatsUpdated(qint64 bytes, double seconds);
    void onOpenFolder();
    void onChangeFolder();
    void onCaptureModeChanged(int index);
    void onReplayEnableToggled(bool checked);
    void onSaveReplay();
    void onCaptureStateChanged(CaptureState state);
    void onQmlSourceAdded(const QString &name, const QString &executableName, const QString &executablePath);
    void onQmlSourceRemoved(const QString &sourceId);
    void updateSourcesView();
    static QVariantList sourcesToVariantList(const QList<AudioSource> &sources);
    void refreshHotkeyMappings();

private:
    QLabel *statusLabel;
    QLabel *timerLabel;
    QLabel *statsLabel;
    QComboBox *appSelector;
    QPushButton *startBtn;
    QPushButton *stopBtn;
    QPushButton *playBtn;
    QPushButton *refreshBtn;
    QLineEdit *saveDirEdit;
    QPushButton *openFolderBtn;
    QPushButton *changeFolderBtn;

    QGroupBox *replayGroupBox;
    QCheckBox *replayEnableCb;
    QSpinBox *replayDurationSpin;
    QLabel *replayStatusLabel;
    QPushButton *saveReplayBtn;
    class WaveformWidget *replayWaveformWidget;

    QTimer *sessionRefreshTimer;

    QMediaCaptureSession *session;
    QAudioInput *audioInput;
    QMediaRecorder *recorder;
    QTimer *stopTimer;
    QTimer *updateTimer;
    int remainingSeconds;
    QElapsedTimer m_recordingTimer;

    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    
    QString lastRecordingPath;

    QQuickWidget *m_sourcesWidget;
    QQuickWidget *m_soundboardWidget;
    QDockWidget *m_sourcesDock;
    QDockWidget *m_soundboardDock;
    QmlBackend *m_qmlBackend;
    SettingsManager *m_settings;
    RecordingManager *m_recordingManager;
    SoundboardManager *m_soundboardManager;
    ActionManager *m_actionManager;
    HotkeyManager *m_hotkeyManager;
};
#endif // MAINWINDOW_H
