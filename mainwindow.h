#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>

class QLabel;
class QPushButton;

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

private:
    QLabel *statusLabel;
    QLabel *timerLabel;
    QPushButton *startBtn;
    QPushButton *stopBtn;
    QPushButton *playBtn;

    QMediaCaptureSession *session;
    QAudioInput *audioInput;
    QMediaRecorder *recorder;
    QTimer *stopTimer;
    QTimer *updateTimer;
    int remainingSeconds;

    QMediaPlayer *player;
    QAudioOutput *audioOutput;
};
#endif // MAINWINDOW_H
