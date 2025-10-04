#include "EngineHandler.h"

EngineHandler::EngineHandler(QObject *parent) : QObject(parent)
{
    stockfishProcess = new QProcess(this);

    // Initialize regular expressions for parsing evaluation
    // cp (centipawns): looks for "cp <number>"
    cpRegex.setPattern("cp (-?\\d+)");
    // mate: looks for "mate <number>"
    mateRegex.setPattern("mate (-?\\d+)");

    connect(stockfishProcess, &QProcess::readyReadStandardOutput,
            this, &EngineHandler::readStandardOutput);
    connect(stockfishProcess, &QProcess::readyReadStandardError,
            this, &EngineHandler::readStandardError);
    connect(stockfishProcess, &QProcess::started,
            this, &EngineHandler::processStarted);
    connect(stockfishProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &EngineHandler::processFinished);
    connect(stockfishProcess, &QProcess::errorOccurred,
            this, &EngineHandler::processErrorOccurred);
    connect(this, &EngineHandler::sgn_engineReady,
            this, &EngineHandler::processNextQueuedAnalysis);

    isAnalyzingPositionComplete = false;
    isEngineReady = false;
    m_isStockfishBusy = false;
}

EngineHandler::~EngineHandler()
{
    stopEngine();
}

void EngineHandler::startEngine()
{
    if (stockfishProcess->state() == QProcess::NotRunning) {
        qDebug() << "Starting Stockfish engine from:" << STOCKFISH_PATH;
        stockfishProcess->start(STOCKFISH_PATH);
    } else {
        qDebug() << "Stockfish already running or starting.";
    }
}

void EngineHandler::stopEngine()
{
    if (stockfishProcess->state() != QProcess::NotRunning) {
        qDebug() << "Stopping Stockfish engine.";
        stockfishProcess->write("quit\n");
        stockfishProcess->waitForBytesWritten();
        stockfishProcess->closeWriteChannel();
        stockfishProcess->waitForFinished(2000);
        if (stockfishProcess->state() != QProcess::NotRunning) {
            stockfishProcess->kill();
            qDebug() << "Stockfish force killed.";
        }
    }
}

void EngineHandler::sendCommand(const QString& command)
{
    if (stockfishProcess->state() == QProcess::Running) {
        // qDebug() << "Sending command to Stockfish:" << command;
        stockfishProcess->write((command + "\n").toUtf8());
        stockfishProcess->waitForBytesWritten();
    } else {
        qWarning() << "Cannot send command: Stockfish not running.";
        emit engineError("Engine not running. Please start it first.");
    }
}

void EngineHandler::analyzePosition(const QString& fen, const QString& moves)
{
    if (stockfishProcess->state() != QProcess::Running) {
        qWarning() << "Cannot analyze: Stockfish not running.";
        emit engineError("Engine not running for analysis.");
        return;
    }

    // Stop any previous analysis
    sendCommand("stop");
    // Set the position
    QString positionCommand = "position " + fen;
    if (!moves.isEmpty()) {
        positionCommand += " moves " + moves;
    }
    sendCommand(positionCommand);

    // "go infinite" for continuous analysis until "stop" is sent
    // "go movetime 3000" for 3 seconds of thinking
    sendCommand("go movetime " + ANALYSIS_TIME_STRING); // Start continuous analysis
}

void EngineHandler::uciMovesReceived(QStringList uciList)
{
    QString compoundUciMove;
    for(QString uci : uciList){
        if(compoundUciMove != ""){
            compoundUciMove = compoundUciMove + " " + uci;
        }else{
            compoundUciMove = uci;
        }
        m_uciCumulativeMoves.enqueue(compoundUciMove);
    }
    processNextQueuedAnalysis();
    // emit sgn_uciMovesReady();
}

void EngineHandler::processNextQueuedAnalysis(){
    if(isEngineReady && !m_uciCumulativeMoves.isEmpty() && !m_isStockfishBusy){
        QString nextMove = m_uciCumulativeMoves.dequeue();
        m_isStockfishBusy=true;
        analyzePosition("startpos", nextMove);
    }
}

void EngineHandler::readStandardOutput()
{
    QByteArray data = stockfishProcess->readAllStandardOutput();
    QString output = QString::fromUtf8(data).trimmed();
    // Append to buffer for multi-line parsing if needed, or process line by line
    // For evaluation, we often get multiple 'info' lines, so processing each line is good.
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    static int counter =0;

    for (const QString& line : lines) {
        // qDebug() << "Stockfish Line:" << line;
        emit engineOutputReady(line); // Emit raw line for general logging

        // Parse for evaluation (cp or mate)
        if (line.startsWith("info")) {
            QRegularExpressionMatch cpMatch = cpRegex.match(line);
            if (cpMatch.hasMatch()) {
                currentCp = cpMatch.captured(1).toInt();
                foundCp = true;
            }

            QRegularExpressionMatch mateMatch = mateRegex.match(line);
            if (mateMatch.hasMatch()) {
                currentMate = mateMatch.captured(1).toInt();
                foundMate = true;
            }

            if (foundCp || foundMate) {
            }
        } else if (line.contains("uciok")) {
            sendCommand("isready");
        } else if (line.contains("readyok")) {
            isEngineReady = true;
            emit sgn_engineReady();
        } else if (line.startsWith("bestmove")) {
            // qDebug() << "Best move from engine:" << line;
            if(isEngineReady){

                if (foundCp || foundMate) {
                    normalizedEval = normalizeEvaluation(currentCp, currentMate);
                    qDebug()<<counter<< "cp: "<<currentCp <<" normal: "<< normalizedEval;
                    counter++;
                    emit sgn_newEvaluation(normalizedEval); //sends the new evaluation to boardhandler
                    m_isStockfishBusy = false;
                    processNextQueuedAnalysis();
                }
            }
            // emit evaluationChanged(normalizedEval);
            // You might emit a signal here for the best move to update your board
        }
    }
}

void EngineHandler::readStandardError()
{
    QByteArray data = stockfishProcess->readAllStandardError();
    QString error = QString::fromUtf8(data).trimmed();
    qWarning() << "Stockfish Error:" << error;
    emit engineError(error);
}

void EngineHandler::processStarted()
{
    qDebug() << "Stockfish process started.";
    sendCommand("uci"); // Initialize UCI mode
}

void EngineHandler::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Stockfish process finished with exit code" << exitCode
             << "and status" << exitStatus;
    emit engineError("Engine stopped unexpectedly.");
}

void EngineHandler::processErrorOccurred(QProcess::ProcessError error)
{
    qWarning() << "Stockfish process error:" << error;
    emit engineError("Process error: " + stockfishProcess->errorString());
}

// Helper function to normalize evaluation for the bar
// This is a simplified mapping. Real chess engines can have very high CPs.
// Mate in N is a special case.
int EngineHandler::normalizeEvaluation(int cp, int mate) {
    // Checkmate always results in a full bar
    if (mate != 0) {
        return (mate > 0) ? 100 : -100;
    }

    // A good, standard way to handle centipawns is to use a non-linear scale.
    // The evaluation is most sensitive near 0, and less sensitive as the
    // advantage becomes larger. A simple logarithmic-like scale works well.
    // Values are in centipawns (100 cp = 1 pawn).

    // You can define a maximum cp value for normalization, but
    // a non-linear approach makes it so you don't need a hard clamp.
    const int maxCpForNormalization = 800; // A strong winning position

    int sign = (cp >= 0) ? 1 : -1;
    int absCp = abs(cp);

    // This is a simplified, non-linear mapping. It is not exactly what
    // Chess.com uses but provides a similar feel.
    // It's a logarithmic-like curve that flattens out.
    // It emphasizes small advantages and reduces the impact of huge ones.

    int normalizedValue;
    if (absCp < 100) { // Small advantage (up to 1 pawn)
        normalizedValue = (absCp * 20) / 100; // Map 100 cp to 20%
    } else if (absCp < 400) { // Clear advantage (1 to 4 pawns)
        normalizedValue = 20 + ((absCp - 100) * 30) / 300; // Map 400 cp to 50%
    } else { // Decisive advantage (4+ pawns)
        normalizedValue = 50 + ((absCp - 400) * 50) / maxCpForNormalization; // Map 1200 cp to 100%
        normalizedValue = qBound(0, normalizedValue, 100);
    }

    return sign * normalizedValue;
}
