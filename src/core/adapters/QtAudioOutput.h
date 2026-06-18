#ifndef QTAUDIOOUTPUT_H
#define QTAUDIOOUTPUT_H

#include "core/domain/IAudioOutput.h"
#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

namespace Saiko {
namespace Adapters {

class QtAudioOutput : public QObject, public Saiko::Domain::IAudioOutput {
    Q_OBJECT
public:
    explicit QtAudioOutput(QObject *parent = nullptr);
    void load(const std::string& filePath) override;
    void play() override;
    void stop() override;
    void setVolume(float volume) override;
    std::string state() const override;

private:
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
};

} // namespace Adapters
} // namespace Saiko

#endif // QTAUDIOOUTPUT_H
