#ifndef ROUTINGDIALOG_H
#define ROUTINGDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QAudioDevice>
#include <QAudioSource>
#include <QIODevice>
#include <QVector>
#include "managers/soundboardmanager.h"

class RealtimeWaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RealtimeWaveformWidget(QWidget *parent = nullptr);
    ~RealtimeWaveformWidget();

    void startMonitoring(const QAudioDevice &device);
    void stopMonitoring();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onReadyRead();

private:
    QAudioSource *m_audioSource = nullptr;
    QIODevice *m_audioDevice = nullptr;
    QVector<float> m_samples;
    int m_maxSamples = 400;
};

class RoutingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RoutingDialog(SoundboardManager *manager, QWidget *parent = nullptr);
    ~RoutingDialog() override;

private slots:
    void onVoiceDeviceChanged(int index);

private:
    SoundboardManager *m_manager;
    QCheckBox *m_micCb;
    QCheckBox *m_localCb;
    QCheckBox *m_feedMicCb;
    QComboBox *m_micCombo;
    QComboBox *m_localCombo;
    QComboBox *m_voiceCombo;
    RealtimeWaveformWidget *m_waveformWidget;
};

#endif // ROUTINGDIALOG_H
