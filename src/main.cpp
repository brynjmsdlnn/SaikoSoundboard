#include "ui/qmlbackend.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>

int main(int argc, char *argv[])
{
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QGuiApplication a(argc, argv);
    Q_INIT_RESOURCE(qml);

    QLoggingCategory::setFilterRules("qt.multimedia.ffmpeg.mediacapturesession.warning=false");

    QmlBackend backend;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("qmlBackend", &backend);
    engine.rootContext()->setContextProperty("soundboardSlotModel", backend.slotModel());
    engine.addImageProvider(QLatin1String("fileicon"), new FileIconProvider());

    engine.load(QUrl("qrc:/qml/Main.qml"));

    return QGuiApplication::exec();
}
