#include "ui/waveformitem.h"

WaveformItem::WaveformItem(QQuickItem *parent)
    : QQuickItem(parent)
{
}

void WaveformItem::setStartMs(qint64 ms)
{
    if (m_startMs != ms) {
        m_startMs = ms;
        emit startMsChanged();
    }
}

void WaveformItem::setEndMs(qint64 ms)
{
    if (m_endMs != ms) {
        m_endMs = ms;
        emit endMsChanged();
    }
}

void WaveformItem::setPlayPositionMs(qint64 ms)
{
    if (m_playPositionMs != ms) {
        m_playPositionMs = ms;
        emit playPositionMsChanged();
    }
}

void WaveformItem::setReadOnly(bool ro)
{
    if (m_readOnly != ro) {
        m_readOnly = ro;
        emit readOnlyChanged();
    }
}

void WaveformItem::setClipRange(qint64 startMs, qint64 endMs)
{
    setStartMs(startMs);
    setEndMs(endMs);
}

void WaveformItem::setWaveformData(const QVariant &data)
{
    WaveformData wf = data.value<WaveformData>();
    if (wf.isValid && !wf.peaks.isEmpty()) {
        QVariantList peaks;
        peaks.reserve(wf.peaks.size());
        for (float p : wf.peaks) {
            peaks.append(p);
        }
        m_peaks = peaks;
        m_durationMs = wf.durationMs;
        m_valid = true;
    } else {
        m_peaks.clear();
        m_durationMs = 0;
        m_valid = false;
    }
    emit dataChanged();
}
