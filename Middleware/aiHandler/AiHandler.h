#ifndef AIHANDLER_H
#define AIHANDLER_H

#include <QObject>
#include <QNetworkAccessManager> // For making HTTP requests
#include <QNetworkRequest>     // For HTTP request details
#include <QNetworkReply>       // For HTTP response
#include <QJsonDocument>       // For JSON parsing
#include <QJsonObject>         // For JSON objects
#include <QJsonArray>          // For JSON arrays
#include <QQueue>

struct GameExplanation {
    Q_GADGET
public:
    Q_PROPERTY(int moveIndex MEMBER moveIndex)
    Q_PROPERTY(QString explanation MEMBER explanation)
    
    int moveIndex;
    QString explanation;
};

Q_DECLARE_METATYPE(GameExplanation)

class AiHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<GameExplanation> gameExplanations READ gameExplanations NOTIFY gameExplanationReady)
public:
    explicit AiHandler(QObject *parent = nullptr);
    void newStockfishEvaluationReceived(const int eval);
    void newFenReceived(const QString fen);
    void newMoveReceived(const QString move);
    void initializeAI();
    void explainNextMove();
    void uciMovesReceived(const QStringList);
    void stockfishAnalysisComplete();
    Q_INVOKABLE void requestMoveExplanation(const QString& fenBeforeMove, const QString& moveMade, int evaluation);
    Q_INVOKABLE void requestGameExplanation(const QString& gameAnalysisJson);
    
    // Getter for the gameExplanations property
    QList<GameExplanation> gameExplanations() const { return m_gameExplanations; }

private:
    QNetworkAccessManager *networkManager; // For AI API calls
    QList<int> m_stockfishEvaluationsList;
    QStringList m_uciMoves;
    QList<QString> m_fenList;
    QList<QString> m_movesList;
    bool m_isGeminiBusy;
    QString createGameJsonQuery();
    
    QList<GameExplanation> m_gameExplanations; // Stores (MoveIndex, Explanation)

signals:
    //Signal for AI explanation
    void moveExplanationReady(const QString& explanation);
    // Signal for when AI request starts/ends (for loading indicators)
    void explanationRequestStatus(bool isLoading);
    void aiError(const QString& error);
    void gameExplanationReady(const QList<GameExplanation>& moveExplanations); // Changed from QPair to GameExplanation

private slots:
    // Slot to handle AI API response
    void onGeminiSingleMoveReply(QNetworkReply* reply);
    void onGeminiGameApiReply(QNetworkReply* reply);

};
#endif // AIHANDLER_H
