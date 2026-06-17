#include "mainwindow.h"
#include "sourcesdock.h"
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
    appSelector->addItem("System Output (Global)", 0);
    
    refreshBtn = new QPushButton("Refresh Apps", this);
    
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
    appLayout->addWidget(new QLabel("Monitor:"));
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
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshAppList);
    connect(openFolderBtn, &QPushButton::clicked, this, &MainWindow::onOpenFolder);
    connect(changeFolderBtn, &QPushButton::clicked, this, &MainWindow::onChangeFolder);
    connect(stopTimer, &QTimer::timeout, this, &MainWindow::onStopRecording);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::onUpdateTimer);
    connect(sessionRefreshTimer, &QTimer::timeout, this, &MainWindow::refreshAppList);
    connect(player, &QMediaPlayer::playbackStateChanged, this, &MainWindow::onPlaybackStateChanged);
    
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

    refreshAppList();
    sessionRefreshTimer->start();

    m_sourcesDock = new SourcesDock(this);
    addDockWidget(Qt::RightDockWidgetArea, m_sourcesDock);

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
}

MainWindow::~MainWindow()
{
    saveSources();
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
    for (const QJsonValue& val : arr) {
        m_sources.append(AudioSource::fromJson(val.toObject()));
    }
}

void MainWindow::saveSources()
{
    QJsonArray arr;
    for (const AudioSource& src : m_sources) {
        arr.append(src.toJson());
    }

    QJsonDocument doc(arr);
    QFile file(getSettingsFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }
}

void MainWindow::refreshAppList()
{
#ifdef Q_OS_WIN
    // Keep track of current selection to restore it if possible
    DWORD currentPid = (DWORD)appSelector->currentData().toULongLong();

    appSelector->clear();
    appSelector->addItem("System Output (Global)", 0);
    
    QFileIconProvider iconProvider;

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
                        WCHAR szPath[MAX_PATH];
                        if (GetModuleFileNameExW(hProcess, NULL, szPath, MAX_PATH)) {
                            QString fullPath = QString::fromWCharArray(szPath);
                            QFileInfo fileInfo(fullPath);
                            QString name = fileInfo.fileName();
                            QString label = name + " (PID: " + QString::number(processId) + ")";
                            
                            appSelector->addItem(iconProvider.icon(fileInfo), label, QVariant::fromValue(qulonglong(processId)));

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
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    lastRecordingPath = saveDirectory + QString("/Recording_%1.wav").arg(timestamp);

    DWORD pid = (DWORD)appSelector->currentData().toULongLong();
    
    QString targetName = appSelector->currentText();
    qDebug() << "UI: Starting loopback recording. Focused app:" << targetName << "(PID:" << pid << ")";
    
    wasapiRecorder->start(lastRecordingPath, pid);
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
