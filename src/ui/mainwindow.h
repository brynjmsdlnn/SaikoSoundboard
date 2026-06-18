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

class QLabel;
class QPushButton;
class QComboBox;
class QLineEdit;
class SourcesDock;
class SoundboardDock;

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

    QList<AudioSource> m_sources;
    SourcesDock *m_sourcesDock;
    SoundboardDock *m_soundboardDock;
    SettingsManager *m_settings;
    RecordingManager *m_recordingManager;
    SoundboardManager *m_soundboardManager;
    ActionManager *m_actionManager;
    HotkeyManager *m_hotkeyManager;
    void *m_hotkeyBackend; // generic pointer or forward-declared pointer to bypass heavy header including if needed, but let's use a forward decl or include.
};
#endif // MAINWINDOW_H
