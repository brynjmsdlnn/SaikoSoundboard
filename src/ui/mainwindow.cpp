#include "ui/mainwindow.h"
#include "ui/sourcesdock.h"
#include "ui/soundboarddock.h"
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
#include <QFileInfo>
#include <QDateTime>
#include <QInputDialog>
#include <QDir>
#include <QSettings>
#include <QFileDialog>
#include <QDesktopServices>
#include <QLineEdit>
#include <QFileIconProvider>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , remainingSeconds(0)
{
    resize(850, 600);

    m_settings = new SettingsManager(this);
    m_settings->load();
    m_sources = m_settings->sources();

    statusLabel = new QLabel("Ready", this);
    timerLabel = new QLabel("", this);
    statsLabel = new QLabel("Size: 0 KB | Time: 0s", this);
    
    appSelector = new QComboBox(this);
    appSelector->addItem("System Output (Global)", "global");
    appSelector->addItem("Multi-Track (Sources Dock)", "multi");
    
    refreshBtn = new QPushButton("Refresh List", this);
    refreshBtn->hide(); 
    
    startBtn = new QPushButton("Start Recording", this);
    stopBtn = new QPushButton("Stop Recording", this);
    stopBtn->setEnabled(false);
    playBtn = new QPushButton("Play Last Recording", this);
    playBtn->setEnabled(false);

    saveDirEdit = new QLineEdit(m_settings->saveDirectory(), this);
    saveDirEdit->setReadOnly(true);
    openFolderBtn = new QPushButton("Open", this);
    changeFolderBtn = new QPushButton("Change...", this);

    auto *layout = new QVBoxLayout();
    layout->addWidget(statusLabel);
    layout->addWidget(timerLabel);
    layout->addWidget(statsLabel);
    
    auto *appLayout = new QHBoxLayout();
    appLayout->addWidget(new QLabel("Capture mode:"));
    appLayout->addWidget(appSelector);
    appLayout->addWidget(refreshBtn);
    layout->addLayout(appLayout);
    
    auto *dirLayout = new QHBoxLayout();
    dirLayout->addWidget(new QLabel("Save to:"));
    dirLayout->addWidget(saveDirEdit, 1);
    dirLayout->addWidget(openFolderBtn);
    dirLayout->addWidget(changeFolderBtn);
    layout->addLayout(dirLayout);

    // Replay Buffer Group Box
    replayGroupBox = new QGroupBox("Replay Buffer", this);
    auto *replayLayout = new QVBoxLayout();
    
    auto *replayTopLayout = new QHBoxLayout();
    replayEnableCb = new QCheckBox("Enable Replay Buffer", this);
    replayDurationSpin = new QSpinBox(this);
    replayDurationSpin->setRange(1, 120);
    replayDurationSpin->setSuffix("s");
    replayDurationSpin->setValue(m_settings->replayDuration());
    
    replayTopLayout->addWidget(replayEnableCb);
    replayTopLayout->addStretch();
    replayTopLayout->addWidget(new QLabel("Duration:"));
    replayTopLayout->addWidget(replayDurationSpin);
    replayLayout->addLayout(replayTopLayout);
    
    auto *replayBottomLayout = new QHBoxLayout();
    replayStatusLabel = new QLabel("Status: Inactive", this);
    saveReplayBtn = new QPushButton("Save Replay", this);
    saveReplayBtn->setEnabled(false);
    
    replayBottomLayout->addWidget(replayStatusLabel);
    replayBottomLayout->addStretch();
    replayBottomLayout->addWidget(saveReplayBtn);
    replayLayout->addLayout(replayBottomLayout);
    
    replayGroupBox->setLayout(replayLayout);
    layout->addWidget(replayGroupBox);

    layout->addWidget(startBtn);
    layout->addWidget(stopBtn);
    layout->addWidget(playBtn);

    auto *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    m_recordingManager = new RecordingManager(m_settings, this);
    m_soundboardManager = new SoundboardManager(m_settings, this);
    m_soundboardManager->loadFromSettings();

    connect(m_recordingManager, &RecordingManager::errorOccurred, this, [this](const QString &msg){
        QMessageBox::critical(this, "Recording Error", msg);
        onStopRecording();
    });
    connect(m_recordingManager, &RecordingManager::stateChanged, this, &MainWindow::onCaptureStateChanged);

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
    connect(openFolderBtn, &QPushButton::clicked, this, &MainWindow::onOpenFolder);
    connect(changeFolderBtn, &QPushButton::clicked, this, &MainWindow::onChangeFolder);
    connect(stopTimer, &QTimer::timeout, this, &MainWindow::onStopRecording);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::onUpdateTimer);
    connect(player, &QMediaPlayer::playbackStateChanged, this, &MainWindow::onPlaybackStateChanged);
    connect(appSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onCaptureModeChanged);
    connect(replayEnableCb, &QCheckBox::toggled, this, &MainWindow::onReplayEnableToggled);
    connect(saveReplayBtn, &QPushButton::clicked, this, &MainWindow::onSaveReplay);
    connect(replayDurationSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val){ 
        m_recordingManager->setReplayDuration(val);
        m_settings->setReplayDuration(val);
        m_settings->save(); 
    });
    
    connect(player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString &errorString){
        QMessageBox::critical(this, "Playback Error", errorString);
        statusLabel->setText("Ready");
        startBtn->setEnabled(true);
        playBtn->setEnabled(true);
    });

    m_sourcesDock = new SourcesDock(this);
    addDockWidget(Qt::RightDockWidgetArea, m_sourcesDock);

    connect(m_sourcesDock, &SourcesDock::sourceAdded, this, [this](const AudioSource& src) {
        m_sources.append(src);
        m_sourcesDock->updateSourceList(m_sources);
        m_settings->setSources(m_sources);
        m_settings->save();
    });

    connect(m_sourcesDock, &SourcesDock::sourceRemoved, this, [this](const QString& id) {
        for (int i = 0; i < m_sources.size(); ++i) {
            if (m_sources[i].id == id) {
                m_sources.removeAt(i);
                break;
            }
        }
        m_sourcesDock->updateSourceList(m_sources);
        m_settings->setSources(m_sources);
        m_settings->save();
    });

    m_sourcesDock->updateSourceList(m_sources);

    m_soundboardDock = new SoundboardDock(m_soundboardManager, this);
    addDockWidget(Qt::BottomDockWidgetArea, m_soundboardDock);

    replayEnableCb->setChecked(m_settings->replayEnabled());
    
    // Initial visibility
    onCaptureModeChanged(appSelector->currentIndex());
    onCaptureStateChanged(m_recordingManager->state());
}

MainWindow::~MainWindow()
{
    if (m_soundboardManager) {
        m_soundboardManager->saveToSettings();
    }
    m_settings->save();
}

void MainWindow::onCaptureModeChanged(int index)
{
    QString mode = appSelector->itemData(index).toString();
    m_sourcesDock->setVisible(mode == "multi");
}

void MainWindow::onReplayEnableToggled(bool checked)
{
    QString mode = appSelector->currentData().toString();
    m_recordingManager->setReplayEnabled(checked, mode);
    
    m_settings->setReplayEnabled(checked);
    m_settings->save();
}

void MainWindow::onCaptureStateChanged(CaptureState state)
{
    bool isIdle = (state == CaptureState::Idle);
    bool isRecording = (state == CaptureState::Recording || state == CaptureState::RecordingAndReplay);
    bool isReplayActive = (state == CaptureState::ReplayOnly || state == CaptureState::RecordingAndReplay);

    // Update Controls
    appSelector->setEnabled(isIdle);
    m_sourcesDock->setLocked(!isIdle);
    
    startBtn->setEnabled(!isRecording);
    stopBtn->setEnabled(isRecording);
    
    replayStatusLabel->setText(isReplayActive ? "Status: Active" : "Status: Inactive");
    saveReplayBtn->setEnabled(isReplayActive);
    
    // Avoid recursion if checked state is already correct
    if (replayEnableCb->isChecked() != isReplayActive) {
        replayEnableCb->setChecked(isReplayActive);
    }

    // Status Text
    if (state == CaptureState::Idle) {
        statusLabel->setText("Ready");
    } else if (state == CaptureState::ReplayOnly) {
        statusLabel->setText("Background Replay Active...");
    } else if (state == CaptureState::Recording) {
        statusLabel->setText("Manual Recording Active...");
    } else if (state == CaptureState::RecordingAndReplay) {
        statusLabel->setText("Recording + Replay Active...");
    }
}

void MainWindow::onSaveReplay()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString path = m_settings->saveDirectory() + QString("/Replay_%1.wav").arg(timestamp);
    
    if (m_recordingManager->saveReplay(path)) {
        statusLabel->setText(QString("Replay Saved: %1").arg(QFileInfo(path).fileName()));
        lastRecordingPath = path;
        playBtn->setEnabled(true);
    } else {
        statusLabel->setText("Failed to save replay or buffer empty");
    }
}

void MainWindow::onStartRecording()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    lastRecordingPath = m_settings->saveDirectory() + QString("/Recording_%1.wav").arg(timestamp);

    QString mode = appSelector->currentData().toString();
    if (!m_recordingManager->isEngineRunning()) {
        m_recordingManager->startEngine(mode);
    }

    if (!m_recordingManager->startRecording(lastRecordingPath)) {
        statusLabel->setText("Failed to start manual recording");
        return;
    }

    remainingSeconds = 10;
    m_recordingTimer.start();
    timerLabel->setText(QString("Time remaining: %1s").arg(remainingSeconds));
    statsLabel->setText("Size: 0 KB | Time: 0.0s");
    
    playBtn->setEnabled(false);
    refreshBtn->setEnabled(false);
    sessionRefreshTimer->stop();
    
    stopTimer->start(10000); 
    updateTimer->start(100);
}

void MainWindow::onStopRecording()
{
    updateTimer->stop(); 
    m_recordingManager->stopRecording();

    QFileInfo fileInfo(lastRecordingPath);
    if (fileInfo.exists() && fileInfo.size() > 100) {
        bool ok;
        QString defaultBaseName = fileInfo.baseName();
        QString newName = QInputDialog::getText(this, "Save Recording",
                                             "Enter name for the recording (leave blank for timestamp):", 
                                             QLineEdit::Normal, "", &ok);
        
        if (ok && !newName.trimmed().isEmpty()) {
            QString dir = fileInfo.absolutePath();
            QString newPath = dir + "/" + newName.trimmed();
            if (!newPath.endsWith(".wav", Qt::CaseInsensitive)) {
                newPath += ".wav";
            }
            
            QFileInfo check(newPath);
            int counter = 1;
            QString finalPath = newPath;
            while (check.exists()) {
                QString base = QFileInfo(newPath).path() + "/" + QFileInfo(newPath).baseName();
                finalPath = QString("%1 (%2).wav").arg(base).arg(counter++);
                check = QFileInfo(finalPath);
            }
            
            if (QFile::rename(lastRecordingPath, finalPath)) {
                lastRecordingPath = finalPath;
            }
        }
        
        statusLabel->setText(QString("Saved: %1").arg(QFileInfo(lastRecordingPath).fileName()));
        playBtn->setEnabled(true);
    } else {
        statusLabel->setText("Recording failed or was empty");
        playBtn->setEnabled(false);
    }

    timerLabel->setText("");
    stopTimer->stop();

    refreshBtn->setEnabled(true);
    sessionRefreshTimer->start();
}

void MainWindow::onUpdateTimer()
{
    double elapsed = m_recordingTimer.elapsed() / 1000.0;
    int remaining = std::max(0, 10 - (int)elapsed);
    timerLabel->setText(QString("Time remaining: %1s").arg(remaining));
    
    onStatsUpdated(m_recordingManager->wavWriter()->size(), elapsed);
}

void MainWindow::onStatsUpdated(qint64 bytes, double seconds)
{
    statsLabel->setText(QString("Size: %1 KB | Time: %2s")
                        .arg(bytes / 1024)
                        .arg(seconds, 0, 'f', 1));
}

void MainWindow::onPlayLastRecording()
{
    if (lastRecordingPath.isEmpty()) return;

    QFileInfo fileInfo(lastRecordingPath);
    if (!fileInfo.exists() || fileInfo.size() <= 100) {
        QMessageBox::warning(this, "Playback Error", "The recording is empty or invalid.");
        return;
    }

    player->setSource(QUrl::fromLocalFile(lastRecordingPath));
    player->play();
    statusLabel->setText(QString("Playing: %1").arg(fileInfo.fileName()));
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

void MainWindow::onOpenFolder()
{
    QDir().mkpath(m_settings->saveDirectory());
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_settings->saveDirectory()));
}

void MainWindow::onChangeFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Save Directory", m_settings->saveDirectory(), 
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        m_settings->setSaveDirectory(dir);
        m_settings->save();
        saveDirEdit->setText(dir);
    }
}
