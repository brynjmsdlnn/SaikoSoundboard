#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAudioDevice>
#include "models/soundplayerslot.h"

class SoundPlayer : public QObject
{
    Q_OBJECT
public:
    explicit SoundPlayer(QObject *parent = nullptr);
    ~SoundPlayer();

    void play();
    void stop();
    void setVolume(float volume);

    void load(const QString &filePath);
    QMediaPlayer::PlaybackState playbackState() const;
    QString filePath() const { return m_filePath; }

    // Routing configuration
    void setRouting(OutputRouting routing);
    void setGlobalOverrides(bool micEnabled, bool localEnabled);
    void setDevices(const QAudioDevice &micDevice, const QAudioDevice &localDevice);
    void setClipRange(qint64 startMs, qint64 endMs);
    void playPreview();

    // Getters for status & testing
    bool shouldPlayMic() const;
    bool shouldPlayLocal() const;
    qint64 startTimeMs() const { return m_startTimeMs; }
    qint64 endTimeMs() const { return m_endTimeMs; }

signals:
    void stateChanged(QMediaPlayer::PlaybackState newState);
    void errorOccurred(QMediaPlayer::Error error, const QString &errorString);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);

private slots:
    void handlePlayerStateChanged(QMediaPlayer::PlaybackState state);
    void handlePositionChanged(qint64 position);

private:
    void applyRoutingAndOverrides();

    QMediaPlayer *m_micPlayer;
    QAudioOutput *m_micOutput;

    QMediaPlayer *m_localPlayer;
    QAudioOutput *m_localOutput;

    QString m_filePath;
    OutputRouting m_routing = OutputRouting::Both;
    bool m_globalMicEnabled = true;
    bool m_globalLocalEnabled = true;
    qint64 m_startTimeMs = 0;
    qint64 m_endTimeMs = -1;
    bool m_isPreviewMode = false;
};

#endif // SOUNDPLAYER_H
