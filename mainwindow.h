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
#include "wasapirecorder.h"
#include "audiosource.h"
#include "replaybuffer.h"
#include "wavwriter.h"
#include "settingsmanager.h"

class QLabel;
class QPushButton;
class QComboBox;

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

private:
    void startCaptureEngine();
    void stopCaptureEngine();
    void startRecorderForPid(DWORD pid, const QString& sourceId, float volume);

    QLabel *statusLabel;
    QLabel *timerLabel;
    QLabel *statsLabel;
    QComboBox *appSelector;
    QPushButton *startBtn;
    QPushButton *stopBtn;
    QPushButton *playBtn;
    QPushButton *refreshBtn;
    class QLineEdit *saveDirEdit;
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
    
    WasapiRecorder *wasapiRecorder;
    ReplayBuffer *m_replayBuffer;
    QString lastRecordingPath;
    bool m_isRecording;

    QList<AudioSource> m_sources;
    class SourcesDock *m_sourcesDock;
    class AudioMixer *m_mixer;
    QList<WasapiRecorder*> m_activeRecorders;
    WavWriter *m_wavWriter;
    SettingsManager *m_settings;
};
#endif // MAINWINDOW_H
