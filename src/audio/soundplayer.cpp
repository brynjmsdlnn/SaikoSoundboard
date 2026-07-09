#include "audio/soundplayer.h"
#include <QUrl>
#include <QDebug>
#include <QFile>


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
    disconnect();
    stop();
}

void SoundPlayer::load(const QString &filePath)
{
    m_filePath = filePath;
    if (filePath.isEmpty() || !QFile::exists(filePath)) {
        m_micPlayer->setSource(QUrl());
        m_localPlayer->setSource(QUrl());
        return;
    }
    QUrl url = QUrl::fromLocalFile(filePath);
    m_micPlayer->setSource(url);
    m_localPlayer->setSource(url);
}

void SoundPlayer::setPlaybackMode(PlaybackMode mode)
{
    m_playbackMode = mode;
}

void SoundPlayer::play()
{
    play(m_playbackMode);
}

void SoundPlayer::play(PlaybackMode mode)
{
    m_playbackMode = mode;
    QString fileLeaf = m_filePath.section('/', -1, -1, QString::SectionIncludeTrailingSep).section('\\', -1, -1);

    if (m_playbackMode == PlaybackMode::ToggleStop) {
        qDebug().nospace() << "[SoundPlayer] play mode=Toggle file=\"" << fileLeaf << "\" alreadyPlaying=" << (playbackState() == QMediaPlayer::PlayingState);
        if (playbackState() == QMediaPlayer::PlayingState) {
            stop();
            return;
        }
        m_remainingLoops = 0;
        playInternal();
    }
    else if (m_playbackMode == PlaybackMode::QueuedSequential) {
        qDebug().nospace() << "[SoundPlayer] play mode=Continuous file=\"" << fileLeaf << "\" alreadyPlaying=" << (playbackState() == QMediaPlayer::PlayingState) << " loops=" << m_remainingLoops;
        if (playbackState() == QMediaPlayer::PlayingState) {
            m_remainingLoops++;
        } else {
            m_remainingLoops = 0;
            playInternal();
        }
    }
    else if (m_playbackMode == PlaybackMode::LayeredCutAll || m_playbackMode == PlaybackMode::LayeredRingOut) {
        bool alreadyPlaying = (playbackState() == QMediaPlayer::PlayingState);
        qDebug().nospace() << "[SoundPlayer] play mode=" << (m_playbackMode == PlaybackMode::LayeredCutAll ? "LayeredCutAll" : "LayeredRingOut")
                           << " file=\"" << fileLeaf << "\" alreadyPlaying=" << alreadyPlaying << " activeTransients=" << m_transientPlayers.size();
        if (alreadyPlaying) {
            SoundPlayer *transient = new SoundPlayer(this);
            transient->load(m_filePath);
            transient->setVolume(m_volume);
            transient->setRouting(m_routing);
            transient->setGlobalOverrides(m_globalMicEnabled, m_globalLocalEnabled);
            transient->setDevices(m_micDevice, m_localDevice);
            transient->setClipRange(m_startTimeMs, m_endTimeMs);

            m_transientPlayers.append(transient);
            qDebug().nospace() << "[SoundPlayer] overlapping spawned transient=" << transient << " totalActive=" << m_transientPlayers.size();

            connect(transient, &SoundPlayer::stateChanged, this, [this, transient](QMediaPlayer::PlaybackState state) {
                qDebug().nospace() << "[SoundPlayer] overlapping transient=" << transient << " state=" << state;
                if (state == QMediaPlayer::StoppedState) {
                    m_transientPlayers.removeOne(transient);
                    transient->deleteLater();
                    qDebug().nospace() << "[SoundPlayer] overlapping transient cleaned up remaining=" << m_transientPlayers.size();
                }
                handlePlayerStateChanged(state);
            });

            // QMediaPlayer ignores setPosition if media hasn't loaded yet.
            // Defer play() until both internal players have loaded their media.
            auto deferredPlay = [transient]() {
                bool micOk = !transient->shouldPlayMic() ||
                    transient->m_micPlayer->mediaStatus() == QMediaPlayer::LoadedMedia ||
                    transient->m_micPlayer->mediaStatus() == QMediaPlayer::BufferedMedia ||
                    transient->m_micPlayer->mediaStatus() == QMediaPlayer::BufferingMedia;
                bool localOk = !transient->shouldPlayLocal() ||
                    transient->m_localPlayer->mediaStatus() == QMediaPlayer::LoadedMedia ||
                    transient->m_localPlayer->mediaStatus() == QMediaPlayer::BufferedMedia ||
                    transient->m_localPlayer->mediaStatus() == QMediaPlayer::BufferingMedia;
                if (micOk && localOk) {
                    QObject::disconnect(transient->m_micPlayer, &QMediaPlayer::mediaStatusChanged, transient, nullptr);
                    QObject::disconnect(transient->m_localPlayer, &QMediaPlayer::mediaStatusChanged, transient, nullptr);
                    transient->play(PlaybackMode::RestartRetrigger);
                }
            };
            connect(transient->m_micPlayer, &QMediaPlayer::mediaStatusChanged, transient, deferredPlay);
            connect(transient->m_localPlayer, &QMediaPlayer::mediaStatusChanged, transient, deferredPlay);
            deferredPlay();
        } else {
            m_remainingLoops = 0;
            playInternal();
        }
    }
    else { // Mode 1 (Restart) or Default
        qDebug().nospace() << "[SoundPlayer] play mode=Restart/Default file=\"" << fileLeaf << "\" alreadyPlaying=" << (playbackState() == QMediaPlayer::PlayingState);
        m_remainingLoops = 0;
        playInternal();
    }
}

void SoundPlayer::playInternal()
{
    m_stoppingInternal = true;
    m_micPlayer->stop();
    m_localPlayer->stop();
    m_stoppingInternal = false;

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
    QString fileLeaf = m_filePath.section('/', -1, -1, QString::SectionIncludeTrailingSep).section('\\', -1, -1);
    qDebug().nospace() << "[SoundPlayer] stop file=\"" << fileLeaf << "\" cleaningTransients=" << m_transientPlayers.size();

    m_remainingLoops = 0;
    m_stoppingInternal = true;
    m_micPlayer->stop();
    m_localPlayer->stop();

    for (SoundPlayer *tp : std::as_const(m_transientPlayers)) {
        tp->stop();
        tp->deleteLater();
    }
    m_transientPlayers.clear();
    m_isPreviewMode = false;
    m_stoppingInternal = false;

    QMediaPlayer::PlaybackState current = playbackState();
    if (current != m_lastOverallState) {
        m_lastOverallState = current;
        emit stateChanged(current);
        if (current == QMediaPlayer::StoppedState) {
            emit positionChanged(-1);
        }
    }
}

void SoundPlayer::setVolume(float volume)
{
    m_volume = volume;
    m_micOutput->setVolume(volume);
    m_localOutput->setVolume(volume);
    for (SoundPlayer *tp : std::as_const(m_transientPlayers)) {
        tp->setVolume(volume);
    }
}

QMediaPlayer::PlaybackState SoundPlayer::playbackState() const
{
    if (m_micPlayer->playbackState() == QMediaPlayer::PlayingState ||
        m_localPlayer->playbackState() == QMediaPlayer::PlayingState) {
        return QMediaPlayer::PlayingState;
    }
    for (const SoundPlayer *tp : std::as_const(m_transientPlayers)) {
        if (tp->playbackState() == QMediaPlayer::PlayingState) {
            return QMediaPlayer::PlayingState;
        }
    }
    if (m_micPlayer->playbackState() == QMediaPlayer::PausedState ||
        m_localPlayer->playbackState() == QMediaPlayer::PausedState) {
        return QMediaPlayer::PausedState;
    }
    for (const SoundPlayer *tp : std::as_const(m_transientPlayers)) {
        if (tp->playbackState() == QMediaPlayer::PausedState) {
            return QMediaPlayer::PausedState;
        }
    }
    return QMediaPlayer::StoppedState;
}

void SoundPlayer::setRouting(OutputRouting routing)
{
    m_routing = routing;
    applyRoutingAndOverrides();
    for (SoundPlayer *tp : std::as_const(m_transientPlayers)) {
        tp->setRouting(routing);
    }
}

void SoundPlayer::setGlobalOverrides(bool micEnabled, bool localEnabled)
{
    m_globalMicEnabled = micEnabled;
    m_globalLocalEnabled = localEnabled;
    applyRoutingAndOverrides();
    for (SoundPlayer *tp : std::as_const(m_transientPlayers)) {
        tp->setGlobalOverrides(micEnabled, localEnabled);
    }
}

void SoundPlayer::setDevices(const QAudioDevice &micDevice, const QAudioDevice &localDevice)
{
    m_micDevice = micDevice;
    m_localDevice = localDevice;
    m_micOutput->setDevice(micDevice);
    m_localOutput->setDevice(localDevice);
    for (SoundPlayer *tp : std::as_const(m_transientPlayers)) {
        tp->setDevices(micDevice, localDevice);
    }
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
    if (m_stoppingInternal) return;

    QMediaPlayer::PlaybackState current = playbackState();
    if (current == QMediaPlayer::StoppedState && m_remainingLoops > 0) {
        m_remainingLoops--;
        playInternal();
        return;
    }

    if (current != m_lastOverallState) {
        m_lastOverallState = current;
        emit stateChanged(current);
        if (current == QMediaPlayer::StoppedState) {
            emit positionChanged(-1);
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

    // Check end boundary clipping
    if (m_endTimeMs != -1 && position >= m_endTimeMs) {
        if (m_remainingLoops > 0) {
            m_remainingLoops--;
            playInternal();
        } else {
            if (m_playbackMode == PlaybackMode::LayeredRingOut && !m_transientPlayers.isEmpty()) {
                // Stop main players but let overlapping transients finish naturally
                m_stoppingInternal = true;
                m_micPlayer->stop();
                m_localPlayer->stop();
                m_stoppingInternal = false;
            } else {
                stop();
            }
        }
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
