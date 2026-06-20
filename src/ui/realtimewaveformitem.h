#ifndef REALTIMEWAVEFORMITEM_H
#define REALTIMEWAVEFORMITEM_H

#include <QQuickItem>
#include <QAudioDevice>
#include <QAudioSource>
#include <QIODevice>
#include <QList>
#include <QVariantList>
#include <QVariant>
#include <QtQml/qqmlregistration.h>

class RealtimeWaveformItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(RealtimeWaveform)
    Q_PROPERTY(QString deviceDescription READ deviceDescription WRITE setDeviceDescription NOTIFY deviceDescriptionChanged)
    Q_PROPERTY(QVariantList samples READ samples NOTIFY samplesChanged)
public:
    explicit RealtimeWaveformItem(QQuickItem *parent = nullptr);
    ~RealtimeWaveformItem();

    QString deviceDescription() const { return m_deviceDescription; }
    QVariantList samples() const;

    Q_INVOKABLE void startMonitoring(const QString &deviceDescription);
    Q_INVOKABLE void stopMonitoring();

signals:
    void deviceDescriptionChanged();
    void samplesChanged();

public slots:
    void setDeviceDescription(const QString &description);

private slots:
    void onReadyRead();

private:
    QString m_deviceDescription;
    QAudioSource *m_audioSource = nullptr;
    QIODevice *m_audioDevice = nullptr;
    QList<float> m_samples;
    int m_maxSamples = 200;
};

#endif // REALTIMEWAVEFORMITEM_H
