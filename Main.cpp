#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext> // Needed for setContextProperty.
#include "BoardHandler.h"
#include "MovesListModel.h"
#include "BoardTypes.h"
#include "EngineHandler.h"
#include "AiHandler.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Get the root context of the QML engine
    QQmlContext *context = engine.rootContext();

    // 1. Instantiate your C++ objects
    BoardHandler *boardHandler = new BoardHandler(&app);
    MovesListModel *movesListModel = new MovesListModel(&app); // <-- Create an instance of the model!
    EngineHandler *engineHandler = new EngineHandler(&app);
    AiHandler *aiHandler = new AiHandler(&app);

    // Register the C++ object with QML
    context->setContextProperty("id_boardHandler", boardHandler);
    context->setContextProperty("movesModel", movesListModel);
    context->setContextProperty("id_engineHandler", engineHandler);
    context->setContextProperty("id_aiHandler", aiHandler);

    // 2. Connect the signal from BoardHandler to the slot in MovesListModel
    // When BoardHandler emits rawMovesListReady, MovesListModel::processMoves will be called.
    QObject::connect(boardHandler, &BoardHandler::rawMovesListReady,
                     movesListModel, &MovesListModel::processMoves);
    QObject::connect(boardHandler, &BoardHandler::sgn_startEngine,
                     engineHandler, &EngineHandler::startEngine);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    QObject::connect(boardHandler, &BoardHandler::sgn_uciMovesReady,
                     engineHandler, &EngineHandler::uciMovesReceived);
    QObject::connect(engineHandler, &EngineHandler::sgn_newEvaluation,
                     boardHandler, &BoardHandler::newEvaluation);

    //Connect to AI to generate the request
    QObject::connect(boardHandler, &BoardHandler::sgn_newUCIMove,
                     aiHandler, &AiHandler::newMoveReceived);
    QObject::connect(engineHandler, &EngineHandler::sgn_newEvaluation,
                     aiHandler, &AiHandler::newEvaluationReceived);
    QObject::connect(boardHandler, &BoardHandler::sgn_newFen,
                     aiHandler, &AiHandler::newFenReceived);

    // Register MoveItem and MovesListModel with QML (these lines are already good)
    qmlRegisterType<BoardTypes::MoveItem>("PGN_movesModule", 1, 0, "MoveItem");
    qmlRegisterType<MovesListModel>("PGN_movesModule", 1, 0, "MovesListModel");

    engine.loadFromModule("Chess", "Main"); // Load the root QML file

    // Check if QML loaded successfully
    if (engine.rootObjects().isEmpty()) {
        qCritical("Failed to load QML module: Chess.Main");
        return -1;
    }

    return app.exec();
}
