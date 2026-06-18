#include "ui/mainwindow.h"

#include <QApplication>
#include <QLoggingCategory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Suppress internal FFmpeg media capture warning logs about partial audio writes
    QLoggingCategory::setFilterRules("qt.multimedia.ffmpeg.mediacapturesession.warning=false");

    MainWindow w;
    w.show();
    return QApplication::exec();
}
