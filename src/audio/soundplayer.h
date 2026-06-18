#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include "domain/IAudioOutput.h"
#include <QMediaPlayer>
#include <QAudioOutput>

class SoundPlayer : public QObject, public Saiko::Domain::IAudioOutput
{
    Q_OBJECT
public:
    explicit SoundPlayer(QObject *parent = nullptr);
    ~SoundPlayer();

    // IAudioOutput Implementation
    void load(const std::string& filePath) override;
    void play() override;
    void stop() override;
    void setVolume(float volume) override;
    std::string state() const override;

    // Original Qt-friendly methods (optional but good for compatibility)
    void load(const QString &filePath);
    QMediaPlayer::PlaybackState playbackState() const;
    QString filePath() const { return m_filePath; }

signals:
    void stateChanged(QMediaPlayer::PlaybackState newState);
    void errorOccurred(QMediaPlayer::Error error, const QString &errorString);

private:
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QString m_filePath;
};

#endif // SOUNDPLAYER_H
