#include "core/adapters/QtAudioOutput.h"

namespace Saiko {
namespace Adapters {

QtAudioOutput::QtAudioOutput(QObject *parent) : QObject(parent) {
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
}

void QtAudioOutput::load(const std::string& filePath) {
    m_player->setSource(QUrl::fromLocalFile(QString::fromStdString(filePath)));
}

void QtAudioOutput::play() {
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->stop();
    }
    m_player->play();
}

void QtAudioOutput::stop() {
    m_player->stop();
}

void QtAudioOutput::setVolume(float volume) {
    m_audioOutput->setVolume(volume);
}

std::string QtAudioOutput::state() const {
    switch (m_player->playbackState()) {
        case QMediaPlayer::PlayingState: return "Playing";
        case QMediaPlayer::PausedState:  return "Paused";
        case QMediaPlayer::StoppedState: return "Stopped";
        default: return "Unknown";
    }
}

} // namespace Adapters
} // namespace Saiko
