#include "audio/soundplayer.h"
#include <QUrl>

SoundPlayer::SoundPlayer(QObject *parent)
    : QObject(parent)
{
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);

    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &SoundPlayer::stateChanged);
    connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString &errorString) {
        emit errorOccurred(error, errorString);
    });
}

SoundPlayer::~SoundPlayer()
{
    m_player->stop();
}

void SoundPlayer::load(const std::string& filePath) {
    load(QString::fromStdString(filePath));
}

void SoundPlayer::load(const QString &filePath)
{
    m_filePath = filePath;
    m_player->setSource(QUrl::fromLocalFile(filePath));
}

void SoundPlayer::play()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->stop();
    }
    m_player->play();
}

void SoundPlayer::stop()
{
    m_player->stop();
}

void SoundPlayer::setVolume(float volume)
{
    m_audioOutput->setVolume(volume);
}

std::string SoundPlayer::state() const {
    switch (m_player->playbackState()) {
        case QMediaPlayer::PlayingState: return "Playing";
        case QMediaPlayer::PausedState:  return "Paused";
        case QMediaPlayer::StoppedState: return "Stopped";
        default: return "Unknown";
    }
}

QMediaPlayer::PlaybackState SoundPlayer::playbackState() const
{
    return m_player->playbackState();
}
