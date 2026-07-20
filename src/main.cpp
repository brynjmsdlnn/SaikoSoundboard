#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>
#include <QtQml>

#include "ui/qmlbackend.h"
#include "ui/realtimewaveformitem.h"
#include "ui/waveformitem.h"
#include "ui/colorediconprovider.h"
#include "ui/fileiconprovider.h"
#include "models/soundplayerslotmodel.h"

#include "core/SingleInstanceGuard.h"

#include "logging/Logging.h"
#include "logging/LogModel.h"

#ifdef Q_OS_WIN
#include "platform/windows/WindowsFramelessWindow.h"
#include <QQuickWindow>
#endif

int main(int argc, char *argv[])
{
    // Force the basic single-threaded render loop to synchronize window position
    // updates and frame rendering on Windows. This eliminates drag lag.
    qputenv("QSG_RENDER_LOOP", "basic");
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");

    QGuiApplication a(argc, argv);

    // Application metadata
    QCoreApplication::setOrganizationName("Saiko Interactive");
    QCoreApplication::setApplicationName("Saiko Soundboard");

    // ── Single-instance guard ────────────────────────────────────────────
    // Uses a hardcoded UUID so the named pipe is unique system-wide.
    SingleInstanceGuard guard(QStringLiteral("SaikoSoundboard-{ba5e5a1k-0s0u-0n0d-0b0a-0r0d00000000}"));
    if (!guard.tryStart()) {
        // Another instance was already running and received our ACTIVATE.
        return 0;
    }
    // ─────────────────────────────────────────────────────────────────────

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

#ifdef Q_OS_WIN
    QQuickWindow *window = qobject_cast<QQuickWindow *>(
        engine.rootObjects().isEmpty() ? nullptr : engine.rootObjects().first());

    if (window) {
        auto *frameless = new WindowsFramelessWindow(window);
        QGuiApplication::instance()->installNativeEventFilter(frameless);

        // Bring window to front when a second instance tries to launch.
        QObject::connect(&guard, &SingleInstanceGuard::activateRequested, window, [window]() {
            window->setWindowStates(window->windowStates() & ~Qt::WindowMinimized);
            window->show();
            window->raise();
            window->requestActivate();
        });
    }
#endif

    return QGuiApplication::exec();
}
