#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// WindowMetrics — compile-time source of truth for custom frameless window
// dimensions. Both the C++ WM_NCHITTEST handler and the QML TitleBar component
// derive their values from the single kWindowMetrics constant, ensuring they
// can never drift apart.
//
// All values are in logical pixels at 96 DPI.  The C++ side applies
// GetDpiForWindow scaling at runtime; QML handles its own scaling via Qt's
// high-DPI support.
// ─────────────────────────────────────────────────────────────────────────────

struct WindowMetrics
{
    int resizeBorder;       ///< Invisible native resize zone thickness (logical px)
    int titleBarHeight;     ///< Custom title bar height (logical px)
    int buttonWidth;        ///< Width of each window control button (logical px)
    int buttonCount;        ///< Number of window control buttons

    /// Total width reserved for window buttons.  Returned as HTCLIENT from
    /// WM_NCHITTEST so QML button MouseAreas receive the events.
    constexpr int buttonExclusionWidth() const { return buttonWidth * buttonCount; }
};

/// Canonical frameless window dimensions — single source of truth.
inline constexpr WindowMetrics kWindowMetrics = {
    /* .resizeBorder  = */ 6,
    /* .titleBarHeight = */ 38,
    /* .buttonWidth   = */ 45,
    /* .buttonCount   = */ 6,
};
