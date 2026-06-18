#include "ui/waveformwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPolygon>
#include <QPen>
#include <QBrush>
#include <algorithm>
#include <cmath>

WaveformWidget::WaveformWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumHeight(55);
    setMouseTracking(true);
}

void WaveformWidget::setWaveformData(const WaveformData &data)
{
    m_data = data;
    update();
}

void WaveformWidget::setClipRange(qint64 startMs, qint64 endMs)
{
    m_startMs = startMs;
    m_endMs = endMs;
    update();
}

void WaveformWidget::setPlayPosition(qint64 positionMs)
{
    m_playPositionMs = positionMs;
    update();
}

void WaveformWidget::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    // Dark premium slate background
    painter.fillRect(rect(), QColor(16, 20, 28));

    qint64 duration = m_data.durationMs > 0 ? m_data.durationMs : 1;
    qint64 currentEnd = m_endMs == -1 ? duration : m_endMs;

    if (!m_data.isValid || m_data.peaks.isEmpty()) {
        painter.setPen(QColor(100, 110, 130));
        painter.drawText(rect(), Qt::AlignCenter, "[No Waveform]");
        return;
    }

    int centerY = (h - 15) / 2;
    int graphH = h - 15;

    // Draw reference line
    painter.setPen(QPen(QColor(32, 40, 52), 1, Qt::DashLine));
    painter.drawLine(0, centerY, w, centerY);

    int numPeaks = m_data.peaks.size();
    double step = static_cast<double>(w) / numPeaks;

    double startPct = static_cast<double>(m_startMs) / duration;
    double endPct = static_cast<double>(currentEnd) / duration;

    int startX = static_cast<int>(startPct * w);
    int endX = static_cast<int>(endPct * w);

    // Draw background highlight for the active region
    painter.fillRect(QRect(startX, 0, std::max(1, endX - startX), graphH), QColor(0, 180, 255, 15));

    // Draw waveform peaks
    for (int i = 0; i < numPeaks; ++i) {
        int x = static_cast<int>(i * step);
        float peak = m_data.peaks[i];
        int peakH = static_cast<int>(peak * (graphH - 10) / 2);

        bool inActiveRegion = (x >= startX && x <= endX);
        QColor peakColor = inActiveRegion ? QColor(0, 180, 255) : QColor(64, 72, 88);

        painter.setPen(QPen(peakColor, 2));
        painter.drawLine(x, centerY - peakH, x, centerY + peakH);
    }

    // Draw start trim marker (green)
    painter.setPen(QPen(QColor(0, 230, 118), 2));
    painter.drawLine(startX, 0, startX, graphH);
    painter.setBrush(QColor(0, 230, 118));
    QPolygon startFlag;
    startFlag << QPoint(startX, 0) << QPoint(startX + 6, 0) << QPoint(startX, 6);
    painter.drawPolygon(startFlag);

    // Draw end trim marker (red)
    painter.setPen(QPen(QColor(255, 61, 0), 2));
    painter.drawLine(endX, 0, endX, graphH);
    painter.setBrush(QColor(255, 61, 0));
    QPolygon endFlag;
    endFlag << QPoint(endX, 0) << QPoint(endX - 6, 0) << QPoint(endX, 6);
    painter.drawPolygon(endFlag);

    // Draw current playback cursor if active (yellow)
    if (m_playPositionMs >= 0) {
        double cursorPct = static_cast<double>(m_playPositionMs) / duration;
        int cursorX = static_cast<int>(cursorPct * w);
        painter.setPen(QPen(QColor(255, 235, 59), 2));
        painter.drawLine(cursorX, 0, cursorX, graphH);
    }

    // Draw timeline labels
    painter.setPen(QColor(140, 150, 170));
    painter.setFont(QFont("Segoe UI", 8));
    
    QString startStr = QString::number(m_startMs / 1000.0, 'f', 1) + "s";
    QString endStr = QString::number(currentEnd / 1000.0, 'f', 1) + "s";
    QString totalStr = QString::number(duration / 1000.0, 'f', 1) + "s";

    painter.drawText(QRect(2, h - 15, 60, 15), Qt::AlignLeft | Qt::AlignVCenter, startStr);
    painter.drawText(QRect(w - 62, h - 15, 60, 15), Qt::AlignRight | Qt::AlignVCenter, totalStr);
    
    double rangeSec = (currentEnd - m_startMs) / 1000.0;
    QString rangeStr = QString("Crop: %1s").arg(rangeSec, 0, 'f', 1);
    painter.drawText(QRect(w/2 - 50, h - 15, 100, 15), Qt::AlignCenter | Qt::AlignVCenter, rangeStr);
}

void WaveformWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_data.isValid || m_data.peaks.isEmpty()) return;

    int w = width();
    qint64 duration = m_data.durationMs > 0 ? m_data.durationMs : 1;
    qint64 currentEnd = m_endMs == -1 ? duration : m_endMs;

    double startPct = static_cast<double>(m_startMs) / duration;
    double endPct = static_cast<double>(currentEnd) / duration;

    int startX = static_cast<int>(startPct * w);
    int endX = static_cast<int>(endPct * w);

    int mx = event->pos().x();
    
    // Choose which handle is closest
    int distToStart = std::abs(mx - startX);
    int distToEnd = std::abs(mx - endX);

    if (distToStart < 15 && distToStart <= distToEnd) {
        m_dragState = DragStart;
        setCursor(Qt::SplitHCursor);
    }
    else if (distToEnd < 15) {
        m_dragState = DragEnd;
        setCursor(Qt::SplitHCursor);
    }
    else {
        m_dragState = DragNone;
    }
}

void WaveformWidget::mouseMoveEvent(QMouseEvent *event)
{
    qint64 duration = m_data.durationMs > 0 ? m_data.durationMs : 1;
    int w = width();
    if (w <= 0) return;

    int mx = event->pos().x();
    double pct = std::clamp(static_cast<double>(mx) / w, 0.0, 1.0);
    qint64 newTimeMs = static_cast<qint64>(pct * duration);

    if (m_dragState == DragStart) {
        qint64 currentEnd = m_endMs == -1 ? duration : m_endMs;
        m_startMs = std::clamp(newTimeMs, 0LL, currentEnd - 50); // Keep at least 50ms interval
        emit trimRangeChanged(m_startMs, m_endMs);
        update();
    }
    else if (m_dragState == DragEnd) {
        m_endMs = std::clamp(newTimeMs, m_startMs + 50, duration);
        emit trimRangeChanged(m_startMs, m_endMs);
        update();
    }
    else {
        // Change cursor to split-h if hovering over marker lines
        double startPct = static_cast<double>(m_startMs) / duration;
        double endPct = (m_endMs == -1 ? duration : m_endMs) / static_cast<double>(duration);
        int startX = static_cast<int>(startPct * w);
        int endX = static_cast<int>(endPct * w);

        if (std::abs(mx - startX) < 10 || std::abs(mx - endX) < 10) {
            setCursor(Qt::SplitHCursor);
        } else {
            unsetCursor();
        }
    }
}

void WaveformWidget::mouseReleaseEvent(QMouseEvent *event)
{
    (void)event;
    if (m_dragState != DragNone) {
        emit trimRangeCommit(m_startMs, m_endMs);
    }
    m_dragState = DragNone;
    unsetCursor();
}
