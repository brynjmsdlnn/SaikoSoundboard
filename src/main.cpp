#include "ui/qmlbackend.h"
#include "ui/realtimewaveformitem.h"
#include "ui/waveformitem.h"
#include "ui/colorediconprovider.h"
#include "ui/fileiconprovider.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>
#include <QtQml>
#include "models/soundplayerslotmodel.h"

#include "logging/Logging.h"
#include "logging/LogModel.h"

int main(int argc, char *argv[])
{
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QGuiApplication a(argc, argv);

    // Initialize logging
#ifdef NDEBUG
    Saiko::Logging::initialize(Saiko::Logging::LogLevel::Info);
#else
    Saiko::Logging::initialize(Saiko::Logging::LogLevel::Trace);
#endif

    QLoggingCategory::setFilterRules("qt.multimedia.ffmpeg*=false");

    qmlRegisterType<RealtimeWaveformItem>("Saiko", 1, 0, "RealtimeWaveform");
    qmlRegisterType<WaveformItem>("Saiko", 1, 0, "WaveformData");

    QmlBackend backend;

    qmlRegisterSingletonInstance("Saiko", 1, 0, "Backend", &backend);
    qmlRegisterSingletonInstance("Saiko", 1, 0, "SlotModel", backend.slotModel());

    Saiko::Logging::LogModel logModel;
    qmlRegisterSingletonInstance("Saiko", 1, 0, "LogModel", &logModel);

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("fileicon"), new FileIconProvider());
    engine.addImageProvider(QLatin1String("icons"), new ColoredIconProvider());

    engine.load(QUrl("qrc:/qt/qml/Saiko/src/qml/Main.qml"));

    return QGuiApplication::exec();
}
