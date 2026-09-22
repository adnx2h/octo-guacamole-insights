#include "EngineHandler.h"
#include <QFile>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

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
        const QString path = stockfishPath();
        qDebug() << "Starting Stockfish engine from:" << path;
        if (path.isEmpty()) {
            emit engineError("Android Stockfish could not be prepared.");
            return;
        }
        stockfishProcess->start(path);
    } else {
        qDebug() << "Stockfish already running or starting.";
    }
}

QString EngineHandler::stockfishPath() const
{
#ifdef Q_OS_ANDROID
    const QJniObject context = QJniObject::callStaticObjectMethod(
        "android/app/ActivityThread", "currentApplication",
        "()Landroid/app/Application;");
    if (!context.isValid()) {
        qWarning() << "Unable to obtain the Android application context.";
        return {};
    }
    const QJniObject applicationInfo = context.callObjectMethod(
        "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
    const QJniObject nativeLibraryDir = applicationInfo.getObjectField<jstring>(
        "nativeLibraryDir");
    const QString path = nativeLibraryDir.toString() + "/libstockfish.so";
    if (!QFile::exists(path)) {
        qWarning() << "Android Stockfish was not deployed at:" << path;
        return {};
    }
    return path;
#else
    return QStringLiteral("D:\\bin\\stockfish_x86-64.exe");
#endif
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

void EngineHandler::analyzePosition(const QString &fen, const QString &moves)
{
    // Reset position state
    foundCp = false;
    foundMate = false;
    currentCp = 0;
    currentMate = 0;

    if (stockfishProcess->state() != QProcess::Running)
    {
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
    if(isEngineReady && !m_uciCumulativeMoves.isEmpty() && !m_isStockfishBusy) {
        // Store current move string so readStandardOutput can safely calculate side to move
        m_currentAnalyzingMove = m_uciCumulativeMoves.dequeue(); 
        m_isStockfishBusy = true;
        analyzePosition("startpos", m_currentAnalyzingMove);
    }
    else if(m_uciCumulativeMoves.isEmpty()){
        qDebug()<<"Stockfish analisis complete";
        emit sgn_stockfishAnalysisComplete();
    }
    else{
        qDebug()<<"Stockfish not ready or busy";
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
        if (line.startsWith("info") && line.contains(" score ")) {
            
            bool isWhiteToMove = true;
            if (!m_currentAnalyzingMove.trimmed().isEmpty()) {
                int plyCount = m_currentAnalyzingMove.trimmed().split(' ', Qt::SkipEmptyParts).size();
                isWhiteToMove = (plyCount % 2 == 0); 
            }

            QRegularExpressionMatch cpMatch = cpRegex.match(line);
            if (cpMatch.hasMatch()) {
                int rawCp = cpMatch.captured(1).toInt();
                currentCp = isWhiteToMove ? rawCp : -rawCp;
                foundCp = true;
                foundMate = false;
                currentMate = 0;
            }

            QRegularExpressionMatch mateMatch = mateRegex.match(line);
            if (mateMatch.hasMatch()) {
                int rawMate = mateMatch.captured(1).toInt();
                currentMate = isWhiteToMove ? rawMate : -rawMate;
                foundMate = true;
                foundCp = false;
                currentCp = 0;
            }

        } else if (line.contains("uciok")) {
            sendCommand("isready");
        } else if (line.contains("readyok")) {
            isEngineReady = true;
            emit sgn_engineReady();
        } else if (line.startsWith("bestmove")) {
            if (isEngineReady) {

                // If Stockfish returns "bestmove (none)", the position is terminal (Checkmate or Stalemate).
                // If it was checkmate, force currentMate based on the last player who moved.
                if (line.contains("(none)") || (!foundMate && currentCp == 0)) {
                    int plyCount = m_currentAnalyzingMove.trimmed().split(' ', Qt::SkipEmptyParts).size();

                    // If plyCount is odd, White made the last move and delivered checkmate (+1 -> 100)
                    // If plyCount is even, Black made the last move and delivered checkmate (-1 -> -100)
                    currentMate = (plyCount % 2 != 0) ? 1 : -1;
                    foundMate = true;
                    foundCp = false;
                }

                if (foundCp || foundMate) {
                    normalizedEval = normalizeEvaluation(currentCp, currentMate);
                    qDebug()<<counter<< "cp:"<<currentCp << "mate:" << currentMate << "normal:" << normalizedEval;
                    counter++;

                    emit sgn_newEvaluation(normalizedEval);

                    foundCp = false;
                    foundMate = false;
                    currentCp = 0;
                    currentMate = 0;

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
    qWarning() << "Stockfish process error:" << error << stockfishProcess->errorString();
    emit engineError("Process error: " + stockfishProcess->errorString());
}

int EngineHandler::normalizeEvaluation(int cp, int mate)
{
    // Checkmate always results in a full bar (+100 or -100)
    if (mate != 0) {
        return (mate > 0) ? 100 : -100;
    }

    // winningFraction goes from 0.0 (Black winning) to 1.0 (White winning)
    double winningFraction = 1.0 / (1.0 + std::pow(10.0, -static_cast<double>(cp) / 400.0));

    // Scale to range [-100, 100]
    int normalizedValue = static_cast<int>((winningFraction - 0.5) * 200.0);

    return qBound(-100, normalizedValue, 100);
}