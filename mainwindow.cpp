#include "mainwindow.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QMediaFormat>
#include <QUrl>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , remainingSeconds(0)
{
    statusLabel = new QLabel("Ready", this);
    timerLabel = new QLabel("", this);
    startBtn = new QPushButton("Start Recording", this);
    stopBtn = new QPushButton("Stop Recording", this);
    stopBtn->setEnabled(false);
    playBtn = new QPushButton("Play Last Recording", this);
    playBtn->setEnabled(false);

    auto *layout = new QVBoxLayout();
    layout->addWidget(statusLabel);
    layout->addWidget(timerLabel);
    layout->addWidget(startBtn);
    layout->addWidget(stopBtn);
    layout->addWidget(playBtn);

    auto *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // Initialize Multimedia - Recording
    session = new QMediaCaptureSession(this);
    audioInput = new QAudioInput(this);
    recorder = new QMediaRecorder(this);

    session->setAudioInput(audioInput);
    session->setRecorder(recorder);

    // Set format to WAV
    QMediaFormat format;
    format.setFileFormat(QMediaFormat::Wave);
    format.setAudioCodec(QMediaFormat::AudioCodec::Wave);
    recorder->setMediaFormat(format);
    recorder->setQuality(QMediaRecorder::HighQuality);

    // Initialize Multimedia - Playback
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(1.0);

    // Initialize Timers
    stopTimer = new QTimer(this);
    stopTimer->setSingleShot(true);
    updateTimer = new QTimer(this);

    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartRecording);
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::onStopRecording);
    connect(playBtn, &QPushButton::clicked, this, &MainWindow::onPlayLastRecording);
    connect(stopTimer, &QTimer::timeout, this, &MainWindow::onStopRecording);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::onUpdateTimer);
    connect(player, &QMediaPlayer::playbackStateChanged, this, &MainWindow::onPlaybackStateChanged);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onStartRecording()
{
    qDebug() << "Start Recording clicked";
    
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/recording.wav";
    recorder->setOutputLocation(QUrl::fromLocalFile(filePath));
    
    recorder->record();
    statusLabel->setText("Recording...");
    
    remainingSeconds = 5;
    timerLabel->setText(QString("Time remaining: %1s").arg(remainingSeconds));
    
    startBtn->setEnabled(false);
    stopBtn->setEnabled(true);
    playBtn->setEnabled(false);
    
    stopTimer->start(5000); 
    updateTimer->start(1000);
}

void MainWindow::onStopRecording()
{
    if (recorder->recorderState() == QMediaRecorder::RecordingState) {
        qDebug() << "Stop Recording";
        recorder->stop();
        statusLabel->setText("Ready - Saved to Music folder");
        playBtn->setEnabled(true);
    }
    
    timerLabel->setText("");
    stopTimer->stop();
    updateTimer->stop();
    startBtn->setEnabled(true);
    stopBtn->setEnabled(false);
}

void MainWindow::onUpdateTimer()
{
    remainingSeconds--;
    if (remainingSeconds >= 0) {
        timerLabel->setText(QString("Time remaining: %1s").arg(remainingSeconds));
    }
}

void MainWindow::onPlayLastRecording()
{
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/recording.wav";
    player->setSource(QUrl::fromLocalFile(filePath));
    player->play();
    statusLabel->setText("Playing last recording...");
    startBtn->setEnabled(false);
    playBtn->setEnabled(false);
}

void MainWindow::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    if (state == QMediaPlayer::StoppedState) {
        statusLabel->setText("Ready");
        startBtn->setEnabled(true);
        playBtn->setEnabled(true);
    }
}
