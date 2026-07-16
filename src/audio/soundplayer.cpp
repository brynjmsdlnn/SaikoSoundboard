#include "audio/soundplayer.h"
#include "logging/LogMacros.h"
#include <QUrl>
#include <QFile>
#include <QAudioOutput>

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
        LOG_DEBUG(LogCategory::Playback,
                 QStringLiteral("[SoundPlayer] play mode=Toggle file=\"%1\" alreadyPlaying=%2")
                     .arg(fileLeaf)
                     .arg(playbackState() == QMediaPlayer::PlayingState));
        if (playbackState() == QMediaPlayer::PlayingState) {
            stop();
            return;
        }
        updateRemainingLoops(0);
        playInternal();
    }
    else if (m_playbackMode == PlaybackMode::QueuedSequential) {
        LOG_DEBUG(LogCategory::Playback,
                 QStringLiteral("[SoundPlayer] play mode=Continuous file=\"%1\" alreadyPlaying=%2 loops=%3")
                     .arg(fileLeaf)
                     .arg(playbackState() == QMediaPlayer::PlayingState)
                     .arg(m_remainingLoops));
        if (playbackState() == QMediaPlayer::PlayingState) {
            updateRemainingLoops(m_remainingLoops + 1);
        } else {
            updateRemainingLoops(0);
            playInternal();
        }
    }
    else if (m_playbackMode == PlaybackMode::LayeredCutAll || m_playbackMode == PlaybackMode::LayeredRingOut) {
        bool alreadyPlaying = (playbackState() == QMediaPlayer::PlayingState);
        LOG_DEBUG(LogCategory::Playback,
                 QStringLiteral("[SoundPlayer] play mode=%1 file=\"%2\" alreadyPlaying=%3 activeTransients=%4")
                     .arg(m_playbackMode == PlaybackMode::LayeredCutAll ? QStringLiteral("LayeredCutAll") : QStringLiteral("LayeredRingOut"))
                     .arg(fileLeaf)
                     .arg(alreadyPlaying)
                     .arg(m_transientPlayers.size()));
        if (alreadyPlaying) {
            SoundPlayer *transient = new SoundPlayer(this);
            transient->load(m_filePath);
            transient->setVolume(m_volume);
            transient->setRouting(m_routing);
            transient->setGlobalOverrides(m_globalMicEnabled, m_globalLocalEnabled);
            transient->setDevices(m_micDevice, m_localDevice);
            transient->setClipRange(m_startTimeMs, m_endTimeMs);

            m_transientPlayers.append(transient);
            LOG_DEBUG(LogCategory::Playback,
                     QStringLiteral("[SoundPlayer] overlapping spawned transient=0x%1 totalActive=%2")
                         .arg(QString::number(reinterpret_cast<quintptr>(transient), 16))
                         .arg(m_transientPlayers.size()));

            connect(transient, &SoundPlayer::positionChanged, this, [this]() {
                emit layerPositionsChanged();
            });

            connect(transient, &SoundPlayer::stateChanged, this, [this, transient](QMediaPlayer::PlaybackState state) {
                LOG_DEBUG(LogCategory::Playback,
                         QStringLiteral("[SoundPlayer] overlapping transient=0x%1 state=%2")
                             .arg(QString::number(reinterpret_cast<quintptr>(transient), 16))
                             .arg(static_cast<int>(state)));
                if (state == QMediaPlayer::StoppedState) {
                    m_transientPlayers.removeOne(transient);
                    transient->deleteLater();
                    LOG_DEBUG(LogCategory::Playback,
                             QStringLiteral("[SoundPlayer] overlapping transient cleaned up remaining=%1")
                                 .arg(m_transientPlayers.size()));
                    emit layerPositionsChanged();
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
            updateRemainingLoops(0);
            playInternal();
        }
    }
    else { // Mode 1 (Restart) or Default
        LOG_DEBUG(LogCategory::Playback,
                 QStringLiteral("[SoundPlayer] play mode=Restart/Default file=\"%1\" alreadyPlaying=%2")
                     .arg(fileLeaf)
                     .arg(playbackState() == QMediaPlayer::PlayingState));
        updateRemainingLoops(0);
        playInternal();
    }
}

void SoundPlayer::playInternal()
{
    m_isPreviewMode = false;
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
    LOG_DEBUG(LogCategory::Playback,
             QStringLiteral("[SoundPlayer] stop file=\"%1\" cleaningTransients=%2")
                 .arg(fileLeaf)
                 .arg(m_transientPlayers.size()));

    updateRemainingLoops(0);
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
    emit layerPositionsChanged();

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
        updateRemainingLoops(m_remainingLoops - 1);
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
            updateRemainingLoops(m_remainingLoops - 1);
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

qint64 SoundPlayer::position() const
{
    if (m_localPlayer->playbackState() == QMediaPlayer::PlayingState) {
        return m_localPlayer->position();
    }
    if (m_micPlayer->playbackState() == QMediaPlayer::PlayingState) {
        return m_micPlayer->position();
    }
    return -1;
}

QList<qint64> SoundPlayer::activeLayerPositions() const
{
    QList<qint64> positions;
    for (const SoundPlayer *tp : m_transientPlayers) {
        if (tp->playbackState() == QMediaPlayer::PlayingState) {
            qint64 pos = tp->position();
            if (pos >= 0) {
                positions.append(pos);
            }
        }
    }
    return positions;
}

void SoundPlayer::updateRemainingLoops(int count)
{
    if (m_remainingLoops != count) {
        m_remainingLoops = count;
        emit remainingLoopsChanged(m_remainingLoops);
    }
}
