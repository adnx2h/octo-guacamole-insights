#include "AiHandler.h"

AiHandler::AiHandler(QObject *parent)
    : QObject{parent}
{
    networkManager = new QNetworkAccessManager(this);

    // Connect network manager's finished signal to our slot
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &AiHandler::onGeminiApiReply);

    m_isGeminiBusy = false;
    m_evaluationsList.clear();
    m_movesList.clear();
    m_fenList.clear();
}

void AiHandler::requestExplanation(const QString& fenBeforeMove, const QString& moveMade, int evaluation)
{
    emit explanationRequestStatus(true); // Indicate loading

    // Construct the prompt for the AI
    QString evalDescription;
    if (evaluation > 0) {
        evalDescription = QString("White's advantage is %1. ").arg(evaluation);
    } else if (evaluation < 0) {
        evalDescription = QString("Black's advantage is %1. ").arg(qAbs(evaluation));
    } else {
        evalDescription = "The position is roughly equal. ";
    }

    // Convert evaluation to centipawns for better context if it's not a mate
    QString evalCpString;
    if (qAbs(evaluation) == 100) { // Assuming 100/-100 means mate or very strong advantage
        evalCpString = (evaluation > 0 ? "White has a winning advantage." : "Black has a winning advantage.");
    } else {
        // Assuming our normalization maps 50 to 1000cp, so 1 eval unit = 20cp
        int cpValue = evaluation * 20;
        evalCpString = QString("The evaluation is approximately %1 centipawns. ").arg(cpValue);
    }


    QString prompt = QString("I am playing a chess game. The current board position (FEN) is: %1. The last move made was %2. Stockfish evaluates this position as follows: %3. "
                             "Please explain in simple terms why this move (%2) was good or bad, considering the evaluation. Focus on concepts a chess beginner would understand. "
                             "Keep the explanation concise, around 2-3 sentences. Do not include the FEN or move in the response, just the explanation.").arg(fenBeforeMove, moveMade, evalCpString);

    qDebug() << "AI Prompt:" << prompt;

    // Construct the JSON payload for the Gemini API
    QJsonObject userPart;
    userPart["text"] = prompt;

    QJsonArray partsArray;
    partsArray.append(userPart);

    QJsonObject contentObject;
    contentObject["role"] = "user";
    contentObject["parts"] = partsArray;

    QJsonArray contentsArray;
    contentsArray.append(contentObject);

    QJsonObject payload;
    payload["contents"] = contentsArray;

    const QString apiKey = qEnvironmentVariable("geminiApiKey");

    QNetworkRequest request(QUrl("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + apiKey));
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(payload);
    networkManager->post(request, doc.toJson());
}

void AiHandler::onGeminiApiReply(QNetworkReply* reply)
{
    emit explanationRequestStatus(false); // Indicate loading finished

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        m_isGeminiBusy = false;

        if (jsonDoc.isObject()) {
            QJsonObject rootObj = jsonDoc.object();
            if (rootObj.contains("candidates") && rootObj["candidates"].isArray()) {
                QJsonArray candidates = rootObj["candidates"].toArray();
                if (!candidates.isEmpty()) {
                    QJsonObject firstCandidate = candidates.first().toObject();
                    if (firstCandidate.contains("content") && firstCandidate["content"].isObject()) {
                        QJsonObject content = firstCandidate["content"].toObject();
                        if (content.contains("parts") && content["parts"].isArray()) {
                            QJsonArray parts = content["parts"].toArray();
                            if (!parts.isEmpty()) {
                                QJsonObject firstPart = parts.first().toObject();
                                if (firstPart.contains("text") && firstPart["text"].isString()) {
                                    QString explanation = firstPart["text"].toString();
                                    qDebug() << "AI Explanation:" << explanation;
                                    emit moveExplanationReady(explanation);
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        qWarning() << "Gemini API Error:" << reply->errorString();
        emit aiError("AI Explanation Error: " + reply->errorString());
    }
    reply->deleteLater();
}

void AiHandler::newEvaluationReceived(int eval){
    m_evaluationsList.append(eval);
}

void AiHandler::newFenReceived(QString fen){
    m_fenList.append(fen);
}

void AiHandler::newMoveReceived(QString move){
    m_movesList.append(move);
}

void AiHandler::initializeAI(){
    m_evaluationsList.clear();
    m_movesList.clear();
    m_fenList.clear();
    m_isGeminiBusy = false;
}

void AiHandler::explainNextMove(){
    // if(isIaReady && !m_evaluationsList.isEmpty() && !m_isIaBusy){
    if(!m_evaluationsList.isEmpty() && !m_fenList.isEmpty() && !m_movesList.isEmpty()){
        int evaluation = m_evaluationsList.dequeue();
        QString fen = m_fenList.dequeue();
        QString move = m_movesList.dequeue();

        requestExplanation(fen,move,evaluation);
        m_isGeminiBusy=true;
        // analyzePosition("startpos", nextMove);
    }
}
