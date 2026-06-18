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
{
    resize(600, 200);

    QSettings settings("Saiko", "SaikoSoundboard");
    QString defaultDir = QDir::homePath() + "/Recordings/Saiko Soundboard";
    saveDirectory = settings.value("saveDirectory", defaultDir).toString();
    QDir().mkpath(saveDirectory);

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

    saveDirEdit = new QLineEdit(saveDirectory, this);
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
    connect(openFolderBtn, &QPushButton::clicked, this, &MainWindow::onOpenFolder);
    connect(changeFolderBtn, &QPushButton::clicked, this, &MainWindow::onChangeFolder);
    connect(stopTimer, &QTimer::timeout, this, &MainWindow::onStopRecording);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::onUpdateTimer);
    connect(player, &QMediaPlayer::playbackStateChanged, this, &MainWindow::onPlaybackStateChanged);
    connect(appSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onCaptureModeChanged);
    
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
    m_mixedFile = nullptr;

    connect(m_mixer, &AudioMixer::mixedPcmReady, this, [this](const QByteArray &data){
        if (m_mixedFile && m_mixedFile->isOpen()) {
            m_mixedFile->write(data);
        }
    });

    connect(m_sourcesDock, &SourcesDock::sourceAdded, this, [this](const AudioSource& src) {
        m_sources.append(src);
        m_sourcesDock->updateSourceList(m_sources);
        saveSources();
    });

    connect(m_sourcesDock, &SourcesDock::sourceRemoved, this, [this](const QString& id) {
        for (int i = 0; i < m_sources.size(); ++i) {
            if (m_sources[i].id == id) {
                m_sources.removeAt(i);
                break;
            }
        }
        m_sourcesDock->updateSourceList(m_sources);
        saveSources();
    });

    loadSources();
    m_sourcesDock->updateSourceList(m_sources);
    
    // Initial visibility
    onCaptureModeChanged(appSelector->currentIndex());
}

MainWindow::~MainWindow()
{
    saveSources();
}

void MainWindow::onCaptureModeChanged(int index)
{
    QString mode = appSelector->itemData(index).toString();
    m_sourcesDock->setVisible(mode == "multi");
}

QString MainWindow::getSettingsFilePath() const
{
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appData);
    return appData + "/settings.json";
}

void MainWindow::loadSources()
{
    QFile file(getSettingsFilePath());
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray arr = doc.array();

    m_sources.clear();
    for (const QJsonValue& val : std::as_const(arr)) {
        m_sources.append(AudioSource::fromJson(val.toObject()));
    }
}

void MainWindow::saveSources()
{
    QJsonArray arr;
    for (const AudioSource& src : std::as_const(m_sources)) {
        arr.append(src.toJson());
    }

    QJsonDocument doc(arr);
    QFile file(getSettingsFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }
}

void MainWindow::onStartRecording()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    lastRecordingPath = saveDirectory + QString("/Recording_%1.wav").arg(timestamp);

    qDebug() << "UI: Starting multi-source capture orchestration...";
    setupMultiTrackRecording();

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
    qDebug() << "UI: Stopping multi-source capture orchestration...";
    updateTimer->stop(); // Stop updates immediately to freeze stats at end
    cleanupMultiTrackRecording();

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
    updateTimer->stop();

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
    
    qint64 fileSize = m_mixedFile ? m_mixedFile->size() : 0;
    onStatsUpdated(fileSize, elapsed);
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

void MainWindow::setupMultiTrackRecording()
{
    cleanupMultiTrackRecording();
    
    m_mixedFile = new QFile(lastRecordingPath, this);
    if (!m_mixedFile->open(QIODevice::WriteOnly)) {
        delete m_mixedFile;
        m_mixedFile = nullptr;
        return;
    }

    // Default format (will be updated by the first recorder that starts)
    WAVEFORMATEXTENSIBLE format;
    memset(&format, 0, sizeof(format));
    m_mixer->setOutputFormat(format);
    m_mixer->start();

    QString mode = appSelector->currentData().toString();
    
    if (mode == "global") {
        // Option 1: Record everything
        startRecorderForPid(0, "global", 1.0f);
    } else {
        // Option 2: Record specific apps from the dock
        for (const auto& src : std::as_const(m_sources)) {
            if (!src.enabled) continue;

            // Find PID for executableName
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
    
    // First recorder sets the master format for the mixer
    connect(rec, &WasapiRecorder::statsUpdated, this, [this, rec](qint64, double){
        if (m_mixer->getOutputFormat().Format.nSamplesPerSec == 0) {
            WAVEFORMATEXTENSIBLE fmt = rec->getFormat();
            m_mixer->setOutputFormat(fmt);
            // Write header to the file once we have the format
            wasapiRecorder->writeWavHeader(*m_mixedFile, &fmt);
        }
    });

    rec->start(pid);
    m_activeRecorders.append(rec);
}

void MainWindow::cleanupMultiTrackRecording()
{
    m_mixer->stop();
    for (auto rec : std::as_const(m_activeRecorders)) {
        rec->stop();
    }
    m_activeRecorders.clear();

    if (m_mixedFile) {
        if (m_mixedFile->isOpen()) {
            // Update WAV header
            wasapiRecorder->updateWavHeader(*m_mixedFile);
            m_mixedFile->close();
        }
        delete m_mixedFile;
        m_mixedFile = nullptr;
    }
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
    QDir().mkpath(saveDirectory);
    QDesktopServices::openUrl(QUrl::fromLocalFile(saveDirectory));
}

void MainWindow::onChangeFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Save Directory", saveDirectory, 
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        saveDirectory = dir;
        saveDirEdit->setText(saveDirectory);
        QSettings settings("Saiko", "SaikoSoundboard");
        settings.setValue("saveDirectory", saveDirectory);
    }
}
