#ifndef SAIKO_LOGGING_LOGCATEGORY_H
#define SAIKO_LOGGING_LOGCATEGORY_H

namespace Saiko {
namespace Logging {
namespace LogCategory {

// clang-format off
inline constexpr auto Audio      = "Audio";
inline constexpr auto Playback   = "Playback";
inline constexpr auto Recording  = "Recording";
inline constexpr auto Replay     = "Replay";
inline constexpr auto Waveform   = "Waveform";
inline constexpr auto Settings   = "Settings";
inline constexpr auto UI         = "UI";
inline constexpr auto Hotkeys    = "Hotkeys";
inline constexpr auto General    = "General";
// clang-format on

} // namespace LogCategory
} // namespace Logging
} // namespace Saiko

// Namespace alias so callers can write LogCategory::Audio etc.
namespace LogCategory = Saiko::Logging::LogCategory;

#endif // SAIKO_LOGGING_LOGCATEGORY_H
