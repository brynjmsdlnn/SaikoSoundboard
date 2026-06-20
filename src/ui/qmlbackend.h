#ifndef QMLBACKEND_H
#define QMLBACKEND_H

#include <QObject>
#include <QQuickImageProvider>
#include <QPixmap>
#include <QFileInfo>
#include <QImage>
#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif
#include <QUrl>
#include <QTimer>
#include <QElapsedTimer>
#include <QVariant>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QDir>
#include "managers/settingsmanager.h"
#include "managers/recordingmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/actionmanager.h"
#include "managers/hotkeymanager.h"
#include "models/capturestate.h"
#include "models/soundplayerslotmodel.h"
#include "models/audiosourcelistmodel.h"
#include "audio/waveformgenerator.h"

class FileIconProvider : public QQuickImageProvider
{
public:
    FileIconProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        QString filePath = QUrl::fromPercentEncoding(id.toUtf8());
        QSize actualSize = requestedSize.isValid() ? requestedSize : QSize(32, 32);
        if (size) *size = actualSize;

#ifdef Q_OS_WIN
        SHFILEINFOW sfi = {};
        SHGetFileInfoW(reinterpret_cast<const wchar_t *>(filePath.utf16()),
                       0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON);
        if (sfi.hIcon) {
            QPixmap px = QPixmap::fromImage(QImage::fromHICON(sfi.hIcon));
            DestroyIcon(sfi.hIcon);
            return px.scaled(actualSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
#endif
        return QPixmap(actualSize);
    }
};

class QmlBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CaptureState captureState READ captureState NOTIFY captureStateChanged)
    Q_PROPERTY(QVariant replayWaveform READ replayWaveform NOTIFY replayWaveformChanged)
    Q_PROPERTY(SettingsManager* settings READ settings CONSTANT)
    Q_PROPERTY(RecordingManager* recording READ recordingManager CONSTANT)
    Q_PROPERTY(SoundboardManager* soundboard READ soundboardManager CONSTANT)
    Q_PROPERTY(ActionManager* actions READ actionManager CONSTANT)
    Q_PROPERTY(HotkeyManager* hotkeys READ hotkeyManager CONSTANT)
    Q_PROPERTY(SoundPlayerSlotModel* slotModel READ slotModel CONSTANT)
    Q_PROPERTY(AudioSourceListModel* sourceModel READ sourceModel CONSTANT)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStateChanged)
public:
    explicit QmlBackend(QObject *parent = nullptr);
    ~QmlBackend();

    SettingsManager *settings() const { return m_settings; }
    RecordingManager *recordingManager() const { return m_recordingManager; }
    SoundboardManager *soundboardManager() const { return m_soundboardManager; }
    ActionManager *actionManager() const { return m_actionManager; }
    HotkeyManager *hotkeyManager() const { return m_hotkeyManager; }
    SoundPlayerSlotModel *slotModel() const { return m_slotModel; }
    AudioSourceListModel *sourceModel() const { return m_sourceModel; }

    CaptureState captureState() const;
    QVariant replayWaveform() const { return QVariant::fromValue(m_replayWaveform); }
    bool isPlaying() const { return m_isPlaying; }

    Q_INVOKABLE QVariantList getRunningProcesses() const;
    Q_INVOKABLE QVariantList getAudioOutputDevices() const;
    Q_INVOKABLE QVariantList getAudioInputDevices() const;
    Q_INVOKABLE qint64 recordingFileSize() const;
    Q_INVOKABLE void playFile(const QString &path);
    Q_INVOKABLE void stopPlayback();
    Q_INVOKABLE QString renameRecordingFile(const QString &oldPath, const QString &dir, const QString &newName);

signals:
    void captureStateChanged(CaptureState state);
    void replayWaveformChanged();
    void playbackStateChanged();

private slots:
    void updateReplayWaveform();

private:
    QTimer *m_replayWaveformTimer;
    WaveformData m_replayWaveform;
    SettingsManager *m_settings;
    RecordingManager *m_recordingManager;
    SoundboardManager *m_soundboardManager;
    ActionManager *m_actionManager;
    HotkeyManager *m_hotkeyManager;
    SoundPlayerSlotModel *m_slotModel = nullptr;
    AudioSourceListModel *m_sourceModel = nullptr;
    void *m_hotkeyBackend;
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    bool m_isPlaying = false;
};

#endif
