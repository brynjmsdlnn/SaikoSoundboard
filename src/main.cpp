#include "ui/qmlbackend.h"

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
    Q_INIT_RESOURCE(qml);

    QLoggingCategory::setFilterRules("qt.multimedia.ffmpeg.mediacapturesession.warning=false");

    QmlBackend backend;

    qmlRegisterSingletonInstance("Saiko", 1, 0, "Backend", &backend);
    qmlRegisterSingletonInstance("Saiko", 1, 0, "SlotModel", backend.slotModel());

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("fileicon"), new FileIconProvider());

    engine.load(QUrl("qrc:/qml/Main.qml"));

    return QGuiApplication::exec();
}
