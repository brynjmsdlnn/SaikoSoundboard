#ifndef CAPTURESTATE_H
#define CAPTURESTATE_H

namespace SaikoCapture {

Q_NAMESPACE

enum CaptureState {
    Idle,
    ReplayOnly,
    Recording,
    RecordingAndReplay
};
Q_ENUM_NS(CaptureState)

} // namespace SaikoCapture

using SaikoCapture::CaptureState;

#endif // CAPTURESTATE_H
