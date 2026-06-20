#include "ui/qmlbackend.h"
#include "ui/realtimewaveformitem.h"
#include "ui/waveformitem.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>
#include <QtQml>
#include "models/soundplayerslotmodel.h"

int main(int argc, char *argv[])
{
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QGuiApplication a(argc, argv);

    QLoggingCategory::setFilterRules("qt.multimedia.ffmpeg.mediacapturesession.warning=false");

    qmlRegisterType<RealtimeWaveformItem>("Saiko", 1, 0, "RealtimeWaveform");
    qmlRegisterType<WaveformItem>("Saiko", 1, 0, "WaveformData");

    QmlBackend backend;

    qmlRegisterSingletonInstance("Saiko", 1, 0, "Backend", &backend);
    qmlRegisterSingletonInstance("Saiko", 1, 0, "SlotModel", backend.slotModel());

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("fileicon"), new FileIconProvider());

    engine.load(QUrl("qrc:/qt/qml/Saiko/src/qml/Main.qml"));

    return QGuiApplication::exec();
}
