#include "mainwindow.h"
#include "sourcesdock.h"
#include "audiomixer.h"
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
    , m_isRecording(false)
{
    resize(600, 200);

    m_settings = new SettingsManager(this);
    m_settings->load();
    m_sources = m_settings->sources();

    statusLabel = new QLabel("Ready", this);
    timerLabel = new QLabel("", this);
    statsLabel = new QLabel("Size: 0 KB | Time: 0s", this);
    
    appSelector = new QComboBox(this);
    appSelector->addItem("System Output (Global)", "global");
    appSelector->addItem("Multi-Track (Sources Dock)", "multi");
    
    // refreshBtn and sessionRefreshTimer are no longer needed for the main dropdown
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

    wasapiRecorder = new WasapiRecorder(this);
    m_replayBuffer = new ReplayBuffer(this);
    m_replayBuffer->setDuration(m_settings->replayDuration());

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
        m_replayBuffer->setDuration(val);
        m_settings->setReplayDuration(val);
        m_settings->save(); 
    });
    
    connect(player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString &errorString){
        QMessageBox::critical(this, "Playback Error", errorString);
        statusLabel->setText("Ready");
        startBtn->setEnabled(true);
        playBtn->setEnabled(true);
    });

    connect(wasapiRecorder, &WasapiRecorder::error, this, [this](const QString &msg){
        QMessageBox::critical(this, "Recording Error", msg);
        onStopRecording();
    });
    connect(wasapiRecorder, &WasapiRecorder::statsUpdated, this, &MainWindow::onStatsUpdated);
    
    connect(wasapiRecorder, &WasapiRecorder::pcmDataReady, this, [](const QByteArray &data){
        // Test slot to verify PCM data flow
        // qDebug() << "MainWindow: Received PCM chunk of size:" << data.size();
    });

    m_sourcesDock = new SourcesDock(this);
    addDockWidget(Qt::RightDockWidgetArea, m_sourcesDock);

    m_mixer = new AudioMixer(this);
    m_wavWriter = new WavWriter(this);

    connect(m_mixer, &AudioMixer::mixedPcmReady, this, [this](const QByteArray &data){
        if (m_wavWriter->isOpen()) {
            m_wavWriter->writePcm(data);
        }
        if (replayEnableCb->isChecked()) {
            m_replayBuffer->pushPcmChunk(data);
        }
    });

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
    replayEnableCb->setChecked(m_settings->replayEnabled());
    
    // Initial visibility
    onCaptureModeChanged(appSelector->currentIndex());
}

MainWindow::~MainWindow()
{
    m_settings->save();
}

void MainWindow::onCaptureModeChanged(int index)
{
    QString mode = appSelector->itemData(index).toString();
    m_sourcesDock->setVisible(mode == "multi");
}

void MainWindow::onReplayEnableToggled(bool checked)
{
    replayStatusLabel->setText(checked ? "Status: Active" : "Status: Inactive");
    saveReplayBtn->setEnabled(checked);
    
    if (checked) {
        startCaptureEngine();
    } else {
        m_replayBuffer->clear();
        if (!m_isRecording) {
            stopCaptureEngine();
        }
    }
    
    m_settings->setReplayEnabled(checked);
    m_settings->save();
}

void MainWindow::onSaveReplay()
{
    QByteArray data = m_replayBuffer->getBufferData();
    if (data.isEmpty()) {
        statusLabel->setText("Replay Buffer empty");
        return;
    }

    WAVEFORMATEXTENSIBLE fmt = m_mixer->getOutputFormat();
    if (fmt.Format.nSamplesPerSec == 0) {
        statusLabel->setText("Invalid audio format for replay");
        return;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString path = m_settings->saveDirectory() + QString("/Replay_%1.wav").arg(timestamp);
    
    WavWriter replayWriter;
    if (!replayWriter.open(path, fmt)) {
        statusLabel->setText("Failed to save replay");
        return;
    }

    replayWriter.writePcm(data);
    replayWriter.close();

    statusLabel->setText(QString("Replay Saved: %1").arg(QFileInfo(path).fileName()));
    lastRecordingPath = path;
    playBtn->setEnabled(true);
}

void MainWindow::onStartRecording()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    lastRecordingPath = m_settings->saveDirectory() + QString("/Recording_%1.wav").arg(timestamp);

    WAVEFORMATEXTENSIBLE fmt = m_mixer->getOutputFormat();
    if (!m_wavWriter->open(lastRecordingPath, fmt)) {
        statusLabel->setText("Failed to start manual recording");
        return;
    }

    m_isRecording = true;
    startCaptureEngine();

    QString mode = appSelector->currentData().toString();
    if (mode == "global") {
        statusLabel->setText("Recording System Output...");
    } else {
        statusLabel->setText("Recording Multi-Source...");
    }
    
    remainingSeconds = 10;
    m_recordingTimer.start();
    timerLabel->setText(QString("Time remaining: %1s").arg(remainingSeconds));
    statsLabel->setText("Size: 0 KB | Time: 0.0s");
    
    startBtn->setEnabled(false);
    stopBtn->setEnabled(true);
    playBtn->setEnabled(false);
    refreshBtn->setEnabled(false);
    appSelector->setEnabled(false);
    sessionRefreshTimer->stop();
    
    stopTimer->start(10000); 
    updateTimer->start(100);
}

void MainWindow::onStopRecording()
{
    m_isRecording = false;
    updateTimer->stop(); 

    m_wavWriter->close();

    if (!replayEnableCb->isChecked()) {
        stopCaptureEngine();
    }

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
            
            // Handle name collisions by adding (1), (2), etc if necessary
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

    startBtn->setEnabled(true);
    stopBtn->setEnabled(false);
    refreshBtn->setEnabled(true);
    appSelector->setEnabled(true);
    sessionRefreshTimer->start();
}

void MainWindow::onUpdateTimer()
{
    double elapsed = m_recordingTimer.elapsed() / 1000.0;
    int remaining = std::max(0, 10 - (int)elapsed);
    timerLabel->setText(QString("Time remaining: %1s").arg(remaining));
    
    onStatsUpdated(m_wavWriter->size(), elapsed);
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

void MainWindow::startCaptureEngine()
{
    if (!m_activeRecorders.isEmpty()) return; // Already running

    // Lock configuration UI
    appSelector->setEnabled(false);
    m_sourcesDock->setLocked(true);

    // Default format (will be updated by the first recorder that starts)
    WAVEFORMATEXTENSIBLE format;
    memset(&format, 0, sizeof(format));
    m_mixer->setOutputFormat(format);
    m_mixer->start();

    QString mode = appSelector->currentData().toString();
    
    if (mode == "global") {
        startRecorderForPid(0, "global", 1.0f);
    } else {
        for (const auto& src : std::as_const(m_sources)) {
            if (!src.enabled) continue;

            DWORD pid = 0;
#ifdef Q_OS_WIN
            DWORD processes[1024], cbNeeded, cProcesses;
            if (EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
                cProcesses = cbNeeded / sizeof(DWORD);
                for (unsigned int i = 0; i < cProcesses; i++) {
                    if (processes[i] != 0) {
                        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
                        if (hProcess) {
                            WCHAR szPath[MAX_PATH];
                            if (GetModuleFileNameExW(hProcess, NULL, szPath, MAX_PATH)) {
                                QString exeName = QFileInfo(QString::fromWCharArray(szPath)).fileName();
                                if (exeName.compare(src.executableName, Qt::CaseInsensitive) == 0) {
                                    pid = processes[i];
                                    CloseHandle(hProcess);
                                    break;
                                }
                            }
                            CloseHandle(hProcess);
                        }
                    }
                }
            }
#endif
            if (pid != 0) {
                startRecorderForPid(pid, src.id, src.volume);
            }
        }
    }
}

void MainWindow::startRecorderForPid(DWORD pid, const QString& sourceId, float volume)
{
    WasapiRecorder *rec = new WasapiRecorder(this);
    m_mixer->addSource(sourceId, volume);
    
    connect(rec, &WasapiRecorder::pcmDataReady, this, [this, sourceId](const QByteArray &data){
        m_mixer->pushPcmData(sourceId, data);
    });

    connect(rec, &WasapiRecorder::finished, rec, &QObject::deleteLater);
    
    // First recorder sets the master format for the mixer and replay buffer
    connect(rec, &WasapiRecorder::statsUpdated, this, [this, rec](qint64, double){
        if (m_mixer->getOutputFormat().Format.nSamplesPerSec == 0) {
            WAVEFORMATEXTENSIBLE fmt = rec->getFormat();
            m_mixer->setOutputFormat(fmt);
            m_replayBuffer->setFormat(fmt);
            
            // If we are currently recording to a file, open it with the newly discovered format
            if (m_wavWriter->isOpen() && m_wavWriter->size() == 0) {
                m_wavWriter->open(m_wavWriter->fileName(), fmt);
            }
        }
    });

    rec->start(pid);
    m_activeRecorders.append(rec);
}

void MainWindow::stopCaptureEngine()
{
    m_mixer->stop();
    for (auto rec : std::as_const(m_activeRecorders)) {
        rec->stop();
    }
    m_activeRecorders.clear();

    // Unlock configuration UI
    appSelector->setEnabled(true);
    m_sourcesDock->setLocked(false);
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
