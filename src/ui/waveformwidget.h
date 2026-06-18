#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QWidget>
#include "audio/waveformgenerator.h"

class WaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaveformWidget(QWidget *parent = nullptr);

    void setWaveformData(const WaveformData &data);
    void setClipRange(qint64 startMs, qint64 endMs);
    void setPlayPosition(qint64 positionMs);
    void setReadOnly(bool readOnly);

signals:
    void trimRangeChanged(qint64 startMs, qint64 endMs);
    void trimRangeCommit(qint64 startMs, qint64 endMs);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    enum DragState { DragNone, DragStart, DragEnd };

    WaveformData m_data;
    qint64 m_startMs = 0;
    qint64 m_endMs = -1;
    qint64 m_playPositionMs = -1;
    bool m_readOnly = false;

    DragState m_dragState = DragNone;
};

#endif // WAVEFORMWIDGET_H
