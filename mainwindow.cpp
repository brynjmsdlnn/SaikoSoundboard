#include "mainwindow.h"
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QMediaFormat>
#include <QUrl>
#include <QStandardPaths>
#include <QMessageBox>

#ifdef Q_OS_WIN
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <psapi.h>
#include <audioclient.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , remainingSeconds(0)
{
    statusLabel = new QLabel("Ready", this);
    timerLabel = new QLabel("", this);
    statsLabel = new QLabel("Size: 0 KB | Time: 0s", this);
    
    appSelector = new QComboBox(this);
    appSelector->addItem("System Output (Global)", 0);
    
    refreshBtn = new QPushButton("Refresh Apps", this);
    
    startBtn = new QPushButton("Start Recording", this);
    stopBtn = new QPushButton("Stop Recording", this);
    stopBtn->setEnabled(false);
    playBtn = new QPushButton("Play Last Recording", this);
    playBtn->setEnabled(false);

    auto *layout = new QVBoxLayout();
    layout->addWidget(statusLabel);
    layout->addWidget(timerLabel);
    layout->addWidget(statsLabel);
    
    auto *appLayout = new QHBoxLayout();
    appLayout->addWidget(new QLabel("Monitor:"));
    appLayout->addWidget(appSelector);
    appLayout->addWidget(refreshBtn);
    layout->addLayout(appLayout);
    
    layout->addWidget(startBtn);
    layout->addWidget(stopBtn);
    layout->addWidget(playBtn);

    auto *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    wasapiRecorder = new WasapiRecorder(this);

    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);

    stopTimer = new QTimer(this);
    stopTimer->setSingleShot(true);
    updateTimer = new QTimer(this);
    
    sessionRefreshTimer = new QTimer(this);
    sessionRefreshTimer->setInterval(2000);

    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartRecording);
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::onStopRecording);
    connect(playBtn, &QPushButton::clicked, this, &MainWindow::onPlayLastRecording);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshAppList);
    connect(stopTimer, &QTimer::timeout, this, &MainWindow::onStopRecording);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::onUpdateTimer);
    connect(sessionRefreshTimer, &QTimer::timeout, this, &MainWindow::refreshAppList);
    connect(player, &QMediaPlayer::playbackStateChanged, this, &MainWindow::onPlaybackStateChanged);
    
    connect(wasapiRecorder, &WasapiRecorder::error, this, [this](const QString &msg){
        QMessageBox::critical(this, "Recording Error", msg);
        onStopRecording();
    });
    connect(wasapiRecorder, &WasapiRecorder::statsUpdated, this, &MainWindow::onStatsUpdated);

    refreshAppList();
    sessionRefreshTimer->start();
}

MainWindow::~MainWindow() {}

void MainWindow::refreshAppList()
{
#ifdef Q_OS_WIN
    // Keep track of current selection to restore it if possible
    DWORD currentPid = (DWORD)appSelector->currentData().toULongLong();

    appSelector->clear();
    appSelector->addItem("System Output (Global)", 0);

    IMMDeviceEnumerator* deviceEnumerator = NULL;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    if (FAILED(hr)) return;

    IMMDevice* device = NULL;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    deviceEnumerator->Release();
    if (FAILED(hr)) return;

    IAudioSessionManager2* sessionManager = NULL;
    hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&sessionManager);
    device->Release();
    if (FAILED(hr)) return;

    IAudioSessionEnumerator* sessionEnumerator = NULL;
    hr = sessionManager->GetSessionEnumerator(&sessionEnumerator);
    sessionManager->Release();
    if (FAILED(hr)) return;

    int count = 0;
    sessionEnumerator->GetCount(&count);
    for (int i = 0; i < count; i++) {
        IAudioSessionControl* sessionControl = NULL;
        hr = sessionEnumerator->GetSession(i, &sessionControl);
        if (SUCCEEDED(hr)) {
            IAudioSessionControl2* sessionControl2 = NULL;
            hr = sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2);
            if (SUCCEEDED(hr)) {
                DWORD processId = 0;
                sessionControl2->GetProcessId(&processId);

                // Avoid picking up the system idle process or dead sessions
                if (processId != 0 && processId != 4) {
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
                    if (hProcess) {
                        WCHAR processName[MAX_PATH];
                        if (GetModuleBaseNameW(hProcess, NULL, processName, MAX_PATH)) {
                            QString name = QString::fromWCharArray(processName);
                            QString label = name + " (PID: " + QString::number(processId) + ")";
                            appSelector->addItem(label, QVariant::fromValue(qulonglong(processId)));

                            if (processId == currentPid) {
                                appSelector->setCurrentIndex(appSelector->count() - 1);
                            }
                        }
                        CloseHandle(hProcess);
                    }
                }
                sessionControl2->Release();
            }
            sessionControl->Release();
        }
    }
    sessionEnumerator->Release();
#endif
}

void MainWindow::onStartRecording()
{
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/recording.wav";
    DWORD pid = (DWORD)appSelector->currentData().toULongLong();
    
    QString targetName = appSelector->currentText();
    qDebug() << "UI: Starting loopback recording. Focused app:" << targetName << "(PID:" << pid << ")";
    
    wasapiRecorder->start(filePath, pid);
    statusLabel->setText(QString("Recording... (Monitoring: %1)").arg(targetName));
    
    remainingSeconds = 10;
    timerLabel->setText(QString("Time remaining: %1s").arg(remainingSeconds));
    statsLabel->setText("Size: 0 KB | Time: 0.0s");
    
    startBtn->setEnabled(false);
    stopBtn->setEnabled(true);
    playBtn->setEnabled(false);
    refreshBtn->setEnabled(false);
    appSelector->setEnabled(false);
    sessionRefreshTimer->stop();
    
    stopTimer->start(10000); 
    updateTimer->start(1000);
}

void MainWindow::onStopRecording()
{
    // Unconditionally safe to call; does nothing if already stopped
    wasapiRecorder->stop();

    statusLabel->setText("Ready - Saved to Music folder");
    playBtn->setEnabled(true);

    timerLabel->setText("");
    stopTimer->stop();
    updateTimer->stop();

    startBtn->setEnabled(true);
    stopBtn->setEnabled(false);
    refreshBtn->setEnabled(true);
    appSelector->setEnabled(true);
    sessionRefreshTimer->start();
}

void MainWindow::onUpdateTimer()
{
    remainingSeconds--;
    if (remainingSeconds >= 0) {
        timerLabel->setText(QString("Time remaining: %1s").arg(remainingSeconds));
    }
}

void MainWindow::onStatsUpdated(qint64 bytes, double seconds)
{
    statsLabel->setText(QString("Size: %1 KB | Time: %2s")
                        .arg(bytes / 1024)
                        .arg(seconds, 0, 'f', 1));
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
