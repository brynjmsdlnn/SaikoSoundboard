#pragma once

#include <QAbstractNativeEventFilter>
#include <QQuickWindow>
#include "../WindowMetrics.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#endif

// ─────────────────────────────────────────────────────────────────────────────
// WindowsFramelessWindow — custom frameless window implementation for Windows.
//
// Responsibilities:
//   • Remove native window chrome (title bar, borders) via WM_NCCALCSIZE
//   • Provide hit-testing via WM_NCHITTEST so Aero Snap, resize, dragging,
//     and shadows all work without native chrome
//   • Enforce minimum window size via WM_GETMINMAXINFO
//   • DPI-scale all hit-test coordinates using GetDpiForWindow
//
// Instantiate once after the QQuickWindow is created and install via
// QGuiApplication::installNativeEventFilter().
// ─────────────────────────────────────────────────────────────────────────────

class WindowsFramelessWindow : public QAbstractNativeEventFilter
{
public:
    explicit WindowsFramelessWindow(QQuickWindow *window,
                                     WindowMetrics metrics = kWindowMetrics);
    ~WindowsFramelessWindow() override = default;

    // ── QAbstractNativeEventFilter ──────────────────────────────────────────
    bool nativeEventFilter(const QByteArray &eventType,
                           void *message,
                           qintptr *result) override;

    /// Canonical title bar height, exposed to QML via Backend.titleBarHeight.
    int titleBarHeight() const { return m_metrics.titleBarHeight; }

private:
    // ── Initialization helpers (called from constructor) ────────────────────
    void applyWindowStyles(HWND hwnd);
    void setupDwmComposition(HWND hwnd);

    // ── Per-message handlers ────────────────────────────────────────────────
    bool handleNcCalcSize(MSG *msg, qintptr *result);
    bool handleNcHitTest(MSG *msg, qintptr *result);
    bool handleGetMinMaxInfo(MSG *msg, qintptr *result);

    // ── DPI-scaling helpers ─────────────────────────────────────────────────
    int dpiScale(HWND hwnd, int value) const;
    int scaledResizeBorder(HWND hwnd) const;
    int scaledTitleBarHeight(HWND hwnd) const;
    int scaledButtonExclusionWidth(HWND hwnd) const;

    // ── Data ────────────────────────────────────────────────────────────────
    QQuickWindow *m_window = nullptr;
    WindowMetrics m_metrics;
};
