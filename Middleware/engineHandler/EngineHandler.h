// EngineHandler.h
#ifndef ENGINEHANDLER_H
#define ENGINEHANDLER_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QStringList>
#include <QRegularExpression> // For parsing output
#include <QQueue>
#include <QThread>

#include <QWaitCondition>

class EngineHandler : public QObject
{
    Q_OBJECT
public:
    EngineHandler(QObject *parent = nullptr);
    ~EngineHandler();

    Q_INVOKABLE void startEngine();
    Q_INVOKABLE void stopEngine();
    Q_INVOKABLE void sendCommand(const QString& command);
    Q_INVOKABLE void analyzePosition(const QString& fen, const QString& moves); // New method to start analysis

    void uciMovesReceived(QStringList);

    const int ANALYSIS_TIME_VALUE = 500;
    const QString ANALYSIS_TIME_STRING = QString::number(ANALYSIS_TIME_VALUE);
    const int ANALYSIS_TIME = ANALYSIS_TIME_VALUE;

signals:
    void engineOutputReady(const QString& output);
    void sgn_engineReady();
    void engineError(const QString& error);
    // New signal for evaluation: value between -100 (Black wins) and 100 (White wins)
    // -100 to 100 is a good range for a percentage-based bar.
    // 0 means equal.
    void evaluationChanged(int evaluation);
    void sgn_newEvaluation(int);

private slots:
    void readStandardOutput();
    void readStandardError();
    void processStarted();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processErrorOccurred(QProcess::ProcessError error);
    void processNextQueuedAnalysis();

private:
    QProcess *stockfishProcess;
    QStringList outputBuffer; // To accumulate multi-line responses if needed
    // Regular expressions for parsing
    QRegularExpression cpRegex;
    QRegularExpression mateRegex;

    // Helper to convert centipawns/mate to a normalized bar value
    int normalizeEvaluation(int cp, int mate);

    QString const STOCKFISH_PATH = "D:\\bin\\stockfish_x86-64.exe";
    //flag to indicate if engine is currently analyzing a position
    bool isAnalyzingPositionComplete;
    QString uciMoves;
    int normalizedEval;
    bool isEngineReady;
    int currentCp = 0;
    int currentMate = 0;
    bool foundCp = false;
    bool foundMate = false;
    QQueue<QString> m_uciCumulativeMoves;
    bool m_isStockfishBusy;
};
#endif
