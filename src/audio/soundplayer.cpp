#include "audio/soundplayer.h"
#include <QUrl>
#include <QDebug>

SoundPlayer::SoundPlayer(QObject *parent)
    : QObject(parent)
{
    m_micPlayer = new QMediaPlayer(this);
    m_micOutput = new QAudioOutput(this);
    m_micPlayer->setAudioOutput(m_micOutput);

    m_localPlayer = new QMediaPlayer(this);
    m_localOutput = new QAudioOutput(this);
    m_localPlayer->setAudioOutput(m_localOutput);

    // Track state changes to report combined state
    connect(m_micPlayer, &QMediaPlayer::playbackStateChanged, this, &SoundPlayer::handlePlayerStateChanged);
    connect(m_localPlayer, &QMediaPlayer::playbackStateChanged, this, &SoundPlayer::handlePlayerStateChanged);

    // Monitor position changes for clipping and timeline updates
    connect(m_micPlayer, &QMediaPlayer::positionChanged, this, &SoundPlayer::handlePositionChanged);
    connect(m_localPlayer, &QMediaPlayer::positionChanged, this, &SoundPlayer::handlePositionChanged);

    connect(m_micPlayer, &QMediaPlayer::durationChanged, this, &SoundPlayer::durationChanged);

    connect(m_micPlayer, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString &errorString) {
        emit errorOccurred(error, "Mic Player: " + errorString);
    });
    connect(m_localPlayer, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString &errorString) {
        emit errorOccurred(error, "Local Player: " + errorString);
    });
}

SoundPlayer::~SoundPlayer()
{
    m_micPlayer->stop();
    m_localPlayer->stop();
}

void SoundPlayer::load(const QString &filePath)
{
    m_filePath = filePath;
    QUrl url = QUrl::fromLocalFile(filePath);
    m_micPlayer->setSource(url);
    m_localPlayer->setSource(url);
}

void SoundPlayer::play()
{
    stop(); // Reset and align

    bool playMic = shouldPlayMic();
    bool playLocal = shouldPlayLocal();

    if (playMic) {
        m_micPlayer->setPosition(m_startTimeMs);
        m_micPlayer->play();
    }
    if (playLocal) {
        m_localPlayer->setPosition(m_startTimeMs);
        m_localPlayer->play();
    }
}

void SoundPlayer::playPreview()
{
    stop(); // Reset and align
    m_isPreviewMode = true;

    if (shouldPlayLocal()) {
        m_localPlayer->setPosition(m_startTimeMs);
        m_localPlayer->play();
    }
}

void SoundPlayer::stop()
{
    m_micPlayer->stop();
    m_localPlayer->stop();
    m_isPreviewMode = false;
}

void SoundPlayer::setVolume(float volume)
{
    m_micOutput->setVolume(volume);
    m_localOutput->setVolume(volume);
}

QMediaPlayer::PlaybackState SoundPlayer::playbackState() const
{
    if (m_micPlayer->playbackState() == QMediaPlayer::PlayingState ||
        m_localPlayer->playbackState() == QMediaPlayer::PlayingState) {
        return QMediaPlayer::PlayingState;
    }
    if (m_micPlayer->playbackState() == QMediaPlayer::PausedState ||
        m_localPlayer->playbackState() == QMediaPlayer::PausedState) {
        return QMediaPlayer::PausedState;
    }
    return QMediaPlayer::StoppedState;
}

void SoundPlayer::setRouting(OutputRouting routing)
{
    m_routing = routing;
    applyRoutingAndOverrides();
}

void SoundPlayer::setGlobalOverrides(bool micEnabled, bool localEnabled)
{
    m_globalMicEnabled = micEnabled;
    m_globalLocalEnabled = localEnabled;
    applyRoutingAndOverrides();
}

void SoundPlayer::setDevices(const QAudioDevice &micDevice, const QAudioDevice &localDevice)
{
    m_micOutput->setDevice(micDevice);
    m_localOutput->setDevice(localDevice);
}

void SoundPlayer::setClipRange(qint64 startMs, qint64 endMs)
{
    m_startTimeMs = startMs;
    m_endTimeMs = endMs;
    applyRoutingAndOverrides();
}

bool SoundPlayer::shouldPlayMic() const
{
    if (m_isPreviewMode) return false;
    return m_globalMicEnabled && (m_routing == OutputRouting::Both || m_routing == OutputRouting::MicOnly);
}

bool SoundPlayer::shouldPlayLocal() const
{
    if (m_isPreviewMode) return m_globalLocalEnabled;
    return m_globalLocalEnabled && (m_routing == OutputRouting::Both || m_routing == OutputRouting::LocalOnly);
}

void SoundPlayer::handlePlayerStateChanged(QMediaPlayer::PlaybackState state)
{
    (void)state;
    static QMediaPlayer::PlaybackState lastOverallState = QMediaPlayer::StoppedState;
    QMediaPlayer::PlaybackState current = playbackState();
    if (current != lastOverallState) {
        lastOverallState = current;
        emit stateChanged(current);
        if (current == QMediaPlayer::StoppedState) {
            emit positionChanged(-1); // Clear UI cursor
        }
    }
}

void SoundPlayer::handlePositionChanged(qint64 position)
{
    QMediaPlayer* senderPlayer = qobject_cast<QMediaPlayer*>(sender());
    if (!senderPlayer) return;

    // Filter duplicate signals: if both players are playing, prefer reporting local player's position
    if (!m_isPreviewMode && 
        m_localPlayer->playbackState() == QMediaPlayer::PlayingState && 
        m_micPlayer->playbackState() == QMediaPlayer::PlayingState && 
        senderPlayer == m_micPlayer) {
        return; 
    }

    // Stop playback immediately when end boundary is reached
    if (m_endTimeMs != -1 && position >= m_endTimeMs) {
        stop();
        return;
    }

    emit positionChanged(position);
}

void SoundPlayer::applyRoutingAndOverrides()
{
    bool playMic = shouldPlayMic();
    bool playLocal = shouldPlayLocal();

    // Check if either player is currently playing
    bool isCurrentlyPlaying = (m_micPlayer->playbackState() == QMediaPlayer::PlayingState ||
                               m_localPlayer->playbackState() == QMediaPlayer::PlayingState);

    if (isCurrentlyPlaying) {
        // Dynamic Sync: Mic path should play but is stopped/paused
        if (playMic && m_micPlayer->playbackState() != QMediaPlayer::PlayingState) {
            qint64 pos = m_localPlayer->position();
            m_micPlayer->setPosition(pos);
            m_micPlayer->play();
        }
        // Mic path should NOT play but is playing
        else if (!playMic && m_micPlayer->playbackState() == QMediaPlayer::PlayingState) {
            m_micPlayer->stop();
        }

        // Dynamic Sync: Local path should play but is stopped/paused
        if (playLocal && m_localPlayer->playbackState() != QMediaPlayer::PlayingState) {
            qint64 pos = m_micPlayer->position();
            m_localPlayer->setPosition(pos);
            m_localPlayer->play();
        }
        // Local path should NOT play but is playing
        else if (!playLocal && m_localPlayer->playbackState() == QMediaPlayer::PlayingState) {
            m_localPlayer->stop();
        }
    } else {
        // If not actively playing, ensure inactive paths are stopped
        if (!playMic) m_micPlayer->stop();
        if (!playLocal) m_localPlayer->stop();
    }
}
