#include "WindowsFramelessWindow.h"
#include <QGuiApplication>

// ─────────────────────────────────────────────────────────────────────────────
// Construction — delegates to focused initialization helpers
// ─────────────────────────────────────────────────────────────────────────────

WindowsFramelessWindow::WindowsFramelessWindow(QQuickWindow *window,
                                                 WindowMetrics metrics)
    : m_window(window)
    , m_metrics(metrics)
{
    HWND hwnd = (HWND)m_window->winId();

    applyWindowStyles(hwnd);
    setupDwmComposition(hwnd);

    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE
                 | SWP_NOZORDER | SWP_NOACTIVATE);
}

// ─────────────────────────────────────────────────────────────────────────────
// applyWindowStyles — set required window styles
// ─────────────────────────────────────────────────────────────────────────────

void WindowsFramelessWindow::applyWindowStyles(HWND hwnd)
{
    LONG style = GetWindowLongW(hwnd, GWL_STYLE);
    style |= WS_THICKFRAME | WS_CAPTION | WS_SYSMENU
           | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
    SetWindowLongW(hwnd, GWL_STYLE, style);
}

// ─────────────────────────────────────────────────────────────────────────────
// setupDwmComposition — configure DWM visual attributes
//
// Future DWM features (rounded corners, dark mode, Mica, Acrylic,
// system backdrop) should be added here, keeping all DWM configuration
// in a single location.
// ─────────────────────────────────────────────────────────────────────────────

void WindowsFramelessWindow::setupDwmComposition(HWND hwnd)
{
    MARGINS margins = {1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);
}

// ─────────────────────────────────────────────────────────────────────────────
// nativeEventFilter — dispatch to per-message handlers
// ─────────────────────────────────────────────────────────────────────────────

bool WindowsFramelessWindow::nativeEventFilter(const QByteArray &eventType,
                                               void *message,
                                               qintptr *result)
{
    if (eventType != "windows_generic_MSG" || !result)
        return false;

    MSG *msg = static_cast<MSG *>(message);

    if (!m_window)
        return false;

    if (!m_window->handle())
        return false;

    HWND ourHwnd = (HWND)m_window->winId();

    if (msg->hwnd != ourHwnd)
        return false;

    switch (msg->message) {
    case WM_NCCALCSIZE:     return handleNcCalcSize(msg, result);
    case WM_NCHITTEST:      return handleNcHitTest(msg, result);
    case WM_GETMINMAXINFO:  return handleGetMinMaxInfo(msg, result);
    default:                return false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// WM_NCCALCSIZE — hide native window chrome
// ─────────────────────────────────────────────────────────────────────────────

bool WindowsFramelessWindow::handleNcCalcSize(MSG *msg, qintptr *result)
{
    if (msg->wParam == TRUE) {
        *result = 0;
        return true;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// WM_NCHITTEST — hit-test for custom chrome
// ─────────────────────────────────────────────────────────────────────────────

bool WindowsFramelessWindow::handleNcHitTest(MSG *msg, qintptr *result)
{
    HWND hwnd = msg->hwnd;

    POINT pt = { GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam) };
    ScreenToClient(hwnd, &pt);

    RECT rect;
    GetClientRect(hwnd, &rect);

    const int border          = scaledResizeBorder(hwnd);
    const int titleBarHeight  = scaledTitleBarHeight(hwnd);
    const int buttonExclusion = scaledButtonExclusionWidth(hwnd);
    const int corner          = border * 2;

    const bool isMaximized = IsZoomed(hwnd);

    if (!isMaximized) {
        if (pt.y < border && pt.x < corner)        { *result = HTTOPLEFT;      return true; }
        if (pt.y < border && pt.x > rect.right - corner) { *result = HTTOPRIGHT; return true; }
        if (pt.y > rect.bottom - border && pt.x < corner)     { *result = HTBOTTOMLEFT;  return true; }
        if (pt.y > rect.bottom - border && pt.x > rect.right - corner) { *result = HTBOTTOMRIGHT; return true; }
        if (pt.y < border)                          { *result = HTTOP;          return true; }
        if (pt.y > rect.bottom - border)            { *result = HTBOTTOM;       return true; }
        if (pt.x < border)                          { *result = HTLEFT;         return true; }
        if (pt.x > rect.right - border)             { *result = HTRIGHT;        return true; }
    }

    if (pt.y < titleBarHeight) {
        if (pt.x > rect.right - buttonExclusion) {
            *result = HTCLIENT;
            return false;
        }
        *result = HTCAPTION;
        return true;
    }

    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// WM_GETMINMAXINFO — enforce minimum / maximum tracking size
// ─────────────────────────────────────────────────────────────────────────────

bool WindowsFramelessWindow::handleGetMinMaxInfo(MSG *msg, qintptr *result)
{
    auto *mmi = reinterpret_cast<MINMAXINFO *>(msg->lParam);
    if (!mmi)
        return false;

    mmi->ptMinTrackSize.x = m_window->minimumWidth();
    mmi->ptMinTrackSize.y = m_window->minimumHeight();

    HMONITOR monitor = MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
    if (monitor) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfoW(monitor, &mi)) {
            mmi->ptMaxTrackSize.x = (mi.rcWork.right  - mi.rcWork.left);
            mmi->ptMaxTrackSize.y = (mi.rcWork.bottom - mi.rcWork.top);
        }
    }

    *result = 0;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// DPI-scaling helpers
// ─────────────────────────────────────────────────────────────────────────────

int WindowsFramelessWindow::dpiScale(HWND hwnd, int value) const
{
    UINT dpi = GetDpiForWindow(hwnd);
    return MulDiv(value, static_cast<int>(dpi), USER_DEFAULT_SCREEN_DPI);
}

int WindowsFramelessWindow::scaledResizeBorder(HWND hwnd) const
{
    return dpiScale(hwnd, m_metrics.resizeBorder);
}

int WindowsFramelessWindow::scaledTitleBarHeight(HWND hwnd) const
{
    return dpiScale(hwnd, m_metrics.titleBarHeight);
}

int WindowsFramelessWindow::scaledButtonExclusionWidth(HWND hwnd) const
{
    return dpiScale(hwnd, m_metrics.buttonExclusionWidth());
}
