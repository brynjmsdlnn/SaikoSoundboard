#ifndef PLAYBACKDEFINITIONS_H
#define PLAYBACKDEFINITIONS_H

#include <QMetaType>

namespace SaikoOutput {
Q_NAMESPACE
enum OutputRouting {
    Both = 0,
    MicOnly = 1,
    LocalOnly = 2
};
Q_ENUM_NS(OutputRouting)
} // namespace SaikoOutput

using SaikoOutput::OutputRouting;

namespace SaikoPlayback {
Q_NAMESPACE
enum PlayState {
    Stopped = 0,
    Playing = 1,
    Preview = 2
};
Q_ENUM_NS(PlayState)

enum PlaybackMode {
    Default          = 0,
    RestartRetrigger = 1,
    ToggleStop       = 2,
    QueuedSequential = 3,
    LayeredCutAll    = 4,
    LayeredRingOut   = 5
};
Q_ENUM_NS(PlaybackMode)

enum class StopReason {
    Natural = 0,
    User = 1,
    Interrupted = 2,
    Error = 3
};
Q_ENUM_NS(StopReason)
} // namespace SaikoPlayback

using SaikoPlayback::PlayState;
using SaikoPlayback::PlaybackMode;
using SaikoPlayback::StopReason;

#endif // PLAYBACKDEFINITIONS_H
