#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

class SoundPlayer : public QObject
{
    Q_OBJECT
public:
    explicit SoundPlayer(QObject *parent = nullptr);
    ~SoundPlayer();

    void load(const QString &filePath);
    void play();
    void stop();
    void setVolume(float volume); // 0.0 to 1.0

    QMediaPlayer::PlaybackState state() const;
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
