#ifndef CAPTURESTATE_H
#define CAPTURESTATE_H

enum class CaptureState {
    Idle,
    ReplayOnly,
    Recording,
    RecordingAndReplay
};

#endif // CAPTURESTATE_H
