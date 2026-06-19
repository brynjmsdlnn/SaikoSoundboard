#ifndef WAVEFORMITEM_H
#define WAVEFORMITEM_H

#include <QQuickItem>
#include <QVariantList>
#include "audio/waveformgenerator.h"

class WaveformItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QVariantList peaks READ peaks NOTIFY dataChanged)
    Q_PROPERTY(qint64 durationMs READ durationMs NOTIFY dataChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY dataChanged)
    Q_PROPERTY(qint64 startMs READ startMs WRITE setStartMs NOTIFY startMsChanged)
    Q_PROPERTY(qint64 endMs READ endMs WRITE setEndMs NOTIFY endMsChanged)
    Q_PROPERTY(qint64 playPositionMs READ playPositionMs WRITE setPlayPositionMs NOTIFY playPositionMsChanged)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly NOTIFY readOnlyChanged)
public:
    explicit WaveformItem(QQuickItem *parent = nullptr);

    QVariantList peaks() const { return m_peaks; }
    qint64 durationMs() const { return m_durationMs; }
    bool isValid() const { return m_valid; }

    qint64 startMs() const { return m_startMs; }
    void setStartMs(qint64 ms);
    qint64 endMs() const { return m_endMs; }
    void setEndMs(qint64 ms);
    qint64 playPositionMs() const { return m_playPositionMs; }
    void setPlayPositionMs(qint64 ms);
    bool isReadOnly() const { return m_readOnly; }
    void setReadOnly(bool ro);

    Q_INVOKABLE void setClipRange(qint64 startMs, qint64 endMs);
    Q_INVOKABLE void setWaveformData(const QVariant &data);

signals:
    void dataChanged();
    void startMsChanged();
    void endMsChanged();
    void playPositionMsChanged();
    void readOnlyChanged();
    void trimRangeChanged(qint64 startMs, qint64 endMs);
    void trimRangeCommit(qint64 startMs, qint64 endMs);

private:
    QVariantList m_peaks;
    qint64 m_durationMs = 0;
    bool m_valid = false;
    qint64 m_startMs = 0;
    qint64 m_endMs = -1;
    qint64 m_playPositionMs = -1;
    bool m_readOnly = false;
};

#endif
