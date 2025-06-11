#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext> // Needed for setContextProperty.
#include "BoardHandler.h"
#include "MovesListModel.h"
#include "BoardTypes.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Get the root context of the QML engine
    QQmlContext *context = engine.rootContext();

    // 1. Instantiate your C++ objects
    // It's good practice to set a parent for objects that are part of the application hierarchy,
    // so they are automatically deleted when the parent is deleted (e.g., the QGuiApplication).
    BoardHandler *boardHandler = new BoardHandler(&app);
    MovesListModel *movesListModel = new MovesListModel(&app); // <-- Create an instance of your model!

    // Register the C++ object with QML
    context->setContextProperty("id_boardHandler", boardHandler); // Pass the pointer
    context->setContextProperty("movesModel", movesListModel);     // <-- Expose your MovesListModel!

    // 2. Connect the signal from BoardHandler to the slot in MovesListModel
    // When BoardHandler emits rawMovesListReady, MovesListModel::processMoves will be called.
    QObject::connect(boardHandler, &BoardHandler::rawMovesListReady,
                     movesListModel, &MovesListModel::processMoves);

    // Register MoveItem and MovesListModel with QML (these lines are already good)
    qmlRegisterType<BoardTypes::MoveItem>("PGN_movesModule", 1, 0, "MoveItem");
    qmlRegisterType<MovesListModel>("PGN_movesModule", 1, 0, "MovesListModel");

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("Chess", "Main"); // Load the root QML file

    // Check if QML loaded successfully
    if (engine.rootObjects().isEmpty()) {
        qCritical("Failed to load QML module: Chess.Main");
        return -1;
    }

    return app.exec();
}
