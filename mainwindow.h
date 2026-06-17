#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "wasapirecorder.h"

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
    void refreshAppList();
    void onStatsUpdated(qint64 bytes, double seconds);
    void onOpenFolder();
    void onChangeFolder();

private:
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

    QTimer *sessionRefreshTimer;

    QMediaCaptureSession *session;
    QAudioInput *audioInput;
    QMediaRecorder *recorder;
    QTimer *stopTimer;
    QTimer *updateTimer;
    int remainingSeconds;

    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    
    WasapiRecorder *wasapiRecorder;
    QString lastRecordingPath;
    QString saveDirectory;
};
#endif // MAINWINDOW_H
