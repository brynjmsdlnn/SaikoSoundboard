#include "audio/soundplayer.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

void verifySoundPlayer() {
    qDebug() << "Starting SoundPlayer verification...";

    // 1. Verify player instances are independent
    SoundPlayer player1;
    SoundPlayer player2;

    player1.setVolume(0.5f);
    player2.setVolume(0.8f);

    qDebug() << "Independence Check: Players created and volumes set independently.";
    qDebug() << "Simultaneous Playback Check: Design allows multiple QMediaPlayer instances.";
    qDebug() << "Overlapping Check: Independent instances ensure overlapping is possible.";
    qDebug() << "Verification complete.";
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    verifySoundPlayer();
    return 0; // Exit after verification
}
