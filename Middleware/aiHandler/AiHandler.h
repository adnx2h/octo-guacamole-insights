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


class AiHandler : public QObject
{
    Q_OBJECT
public:
    explicit AiHandler(QObject *parent = nullptr);
    void newEvaluationReceived(const int eval);
    void newFenReceived(const QString fen);
    void newMoveReceived(const QString move);
    void initializeAI();
    void explainNextMove();
    Q_INVOKABLE void requestExplanation(const QString& fenBeforeMove, const QString& moveMade, int evaluation);

private:
    QNetworkAccessManager *networkManager; // For AI API calls
    QQueue<int> m_evaluationsList;
    QQueue<QString> m_fenList;
    QQueue<QString> m_movesList;
    bool m_isGeminiBusy;

signals:
    //Signal for AI explanation
    void moveExplanationReady(const QString& explanation);
    // Signal for when AI request starts/ends (for loading indicators)
    void explanationRequestStatus(bool isLoading);
    void aiError(const QString& error);

private slots:
    // Slot to handle AI API response
    void onGeminiApiReply(QNetworkReply* reply);

};
#endif // AIHANDLER_H
