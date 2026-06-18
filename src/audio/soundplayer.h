#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include <QMediaPlayer>
#include <QAudioOutput>

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

signals:
    void stateChanged(QMediaPlayer::PlaybackState newState);
    void errorOccurred(QMediaPlayer::Error error, const QString &errorString);

private:
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QString m_filePath;
};

#endif // SOUNDPLAYER_H
