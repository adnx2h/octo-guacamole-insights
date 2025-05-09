#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "Middleware/engineHandler/EngineHandler.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    EngineHandler engineHandler;
    // Get the root context of the QML engine
    QQmlContext *context = engine.rootContext();

    // Register the C++ object with QML
    context->setContextProperty("id_engineHandler", &engineHandler);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Chess", "Main");

    return app.exec();
}
