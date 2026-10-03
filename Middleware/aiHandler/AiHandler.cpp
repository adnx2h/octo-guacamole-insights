#include "AiHandler.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include "Personas.h"
#define NOT_USE_AI

AiHandler::AiHandler(QObject *parent)
    : QObject{parent}
{
    qRegisterMetaType<GameExplanation>("GameExplanation");
    qRegisterMetaType<QList<GameExplanation>>("QList<GameExplanation>");
    
    networkManager = new QNetworkAccessManager(this);

    // Connect network manager's finished signal to our slot
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &AiHandler::onGeminiGameApiReply);

    m_isGeminiBusy = false;
    m_stockfishEvaluationsList.clear();
    m_movesList.clear();
    m_fenList.clear();
}

void AiHandler::requestMoveExplanation(const QString& fenBeforeMove, const QString& moveMade, int evaluation)
{
    emit sgn_explanationRequestStatus(true); // Indicate loading

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

void AiHandler::requestGameExplanation(const QString& gameAnalysisJson)
{
    if (gameAnalysisJson.isEmpty()) {
        qWarning() << "Cannot request game explanation: Input JSON string is empty.";
        return;
    }

    emit sgn_explanationRequestStatus(true); // Indicate loading

    // The Gemini API requires the prompt/query to be wrapped inside the "text" field
    // of a "user" role object in the 'contents' array.

    // --- 1. Build the Final Gemini API Payload ---

    // a) Put the entire query string into a "text" part
    QJsonObject userPart;
    // The input 'gameAnalysisJson' is the content we want the model to analyze.
    userPart["text"] = gameAnalysisJson;

    QJsonArray partsArray;
    partsArray.append(userPart);

    // b) Wrap the parts into a "user" content object
    QJsonObject contentObject;
    contentObject["role"] = "user";
    contentObject["parts"] = partsArray;

    QJsonArray contentsArray;
    contentsArray.append(contentObject);

    QJsonObject payload;
    payload["contents"] = contentsArray;


    // --- 2. Send the Request ---
#ifdef Q_OS_ANDROID
    const QString apiKey = QStringLiteral("123456"); // Temporary Android test key
#else
    const QString apiKey = qEnvironmentVariable("geminiApiKey");
#endif

    // Note: Using the recommended gemini-2.5-flash model
    QNetworkRequest request(QUrl("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=" + apiKey));

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(payload);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    qDebug() << "Sending full game analysis request to API...";

    // Send the data
    networkManager->post(request, jsonData);
}

void AiHandler::onGeminiSingleMoveReply(QNetworkReply* reply)
{
    emit sgn_explanationRequestStatus(false); // Indicate loading finished

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
        emit sgn_aiError("AI Explanation Error: " + reply->errorString());
    }
    reply->deleteLater();
}

void AiHandler::onGeminiGameApiReply(QNetworkReply* reply)
{
    qDebug() << "onGeminiGameApiReply";
    emit sgn_explanationRequestStatus(false); // Indicate loading finished
    m_gameExplanations.clear();          // Clear previous analysis

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Gemini API Error (Game):" << reply->errorString();
        emit sgn_aiError("Game Explanation Error: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    reply->deleteLater();
    qDebug()<<jsonDoc;

    // 1. Traverse the standard Gemini API wrapper structure
    QJsonObject rootObj = jsonDoc.object();
    QJsonArray candidates = rootObj["candidates"].toArray();

    if (candidates.isEmpty()) {
        qWarning() << "Gemini API Error (Game): No candidates found in response.";
        return;
    }

    // Extract the raw text containing the AI's custom JSON output
    QString rawCommentaryText;
    QJsonObject firstCandidate = candidates.first().toObject();
    QJsonObject content = firstCandidate["content"].toObject();
    QJsonArray parts = content["parts"].toArray();

    if (!parts.isEmpty()) {
        rawCommentaryText = parts.first().toObject()["text"].toString().trimmed();
    }

    if (rawCommentaryText.isEmpty()) {
        qWarning() << "Gemini API Error (Game): Could not extract raw commentary text.";
        return;
    }

    // Remove markdown code blocks if present
    if (rawCommentaryText.startsWith("```json")) {
        rawCommentaryText = rawCommentaryText.mid(7); // Remove "```json"
    } else if (rawCommentaryText.startsWith("```")) {
        rawCommentaryText = rawCommentaryText.mid(3); // Remove "```"
    }
    
    if (rawCommentaryText.endsWith("```")) {
        rawCommentaryText = rawCommentaryText.left(rawCommentaryText.length() - 3); // Remove trailing "```"
    }
    
    rawCommentaryText = rawCommentaryText.trimmed(); // Trim any remaining whitespace

    // 2. Parse the AI's custom JSON (the output we requested in the prompt)
    QJsonDocument commentaryDoc = QJsonDocument::fromJson(rawCommentaryText.toUtf8());
    qDebug() << "Raw Commentary Text:" << rawCommentaryText;
    if (!commentaryDoc.isObject()) {
        qWarning() << "Error: AI response is not a valid JSON object. Got:" << rawCommentaryText;
        emit sgn_aiError("AI response error: Could not parse analysis JSON.");
        return;
    }

    QJsonObject commentaryRoot = commentaryDoc.object();
    QJsonArray commentaryArray = commentaryRoot["analysis_commentary"].toArray();

    // 3. Store the Move-by-Move Explanations
    for (const QJsonValue& value : commentaryArray) {
        if (value.isObject()) {
            QJsonObject moveObj = value.toObject();

            // Note: If m_uciMoves is the source of truth for move index (0-based)
            // you might want to adjust the move_number here.
            // Assuming 'move_number' in JSON is 1-based, we'll store a 0-based index.
            int moveIndex = moveObj["move_number"].toInt() - 1;
            QString explanation = moveObj["explanation"].toString();

            if (!explanation.isEmpty()) {
                m_gameExplanations.append({moveIndex, explanation});
            }
        }
    }

    // 4. Signal readiness and make the stored data available
    qDebug() << "Successfully stored" << m_gameExplanations.size() << "move explanations.";
    //commented out to test locally
    emit sgn_gameExplanationReady(m_gameExplanations);
}
//Receive each new Stockfish Evaluation
void AiHandler::newStockfishEvaluationReceived(int eval){
    m_stockfishEvaluationsList.append(eval);
}

//Analysis is complete, i can start building the query
//For an IA response ill provide move number, evaluation, UCI move and FEN
void AiHandler::stockfishAnalysisComplete(){
    const int moveCount = m_uciMoves.size();
    if (moveCount > 0 && moveCount == m_fenList.size() &&
        moveCount == m_stockfishEvaluationsList.size())
    {
        auto jsonAnalysisGame = createGameJsonQuery();
#ifdef USE_AI
        requestGameExplanation(jsonAnalysisGame);
#endif
    }
    else{
        qWarning() << "Error: Data lists are not synchronized or are empty. Cannot create JSON query.";
    }
}

QString AiHandler::createGameJsonQuery()
{
    int moveCount = m_uciMoves.size();

    // 1. Determine the user's color (e.g., "White" or "Black").
    QString userColor = m_userColor.isEmpty() ? "White" : m_userColor; 
    QString opponentColor = (userColor == "White") ? "Black" : "White";

    QJsonArray movesArray;
    // ... (Iterate and Build the JSON Array 'movesArray')
    for (int i = 0; i < moveCount; ++i)
    {
        // ... (Build individual moveObject) ...
        QJsonObject moveObject;
        moveObject["move_number"] = i + 1;
        moveObject["color"] = (i % 2 == 0) ? "White" : "Black";
        moveObject["uci_move"] = m_uciMoves.at(i);
        moveObject["fen_before_move"] = m_fenList.at(i);

        int rawEval = m_stockfishEvaluationsList.at(i);
        moveObject["evaluation_cp"] = rawEval;

        movesArray.append(moveObject);
    }

    // 2. Format the persona prompt with user/opponent colors
    QString formattedPersona = m_persona.arg(m_userColor, m_user);
    qDebug() << formattedPersona;
    QString geminiPrompt = formattedPersona + "\n\n" + JSON_OUTPUT_FORMAT;

    // 3. Create the Final JSON Document
    QJsonObject queryRoot;
    queryRoot["analysis_request_type"] = "commentary";
    queryRoot["user_played_as"] = userColor; // Help the AI know who the user is
    queryRoot["prompt_instructions"] = geminiPrompt;
    queryRoot["starting_fen"] = m_fenList.first();
    queryRoot["total_moves"] = moveCount;
    queryRoot["moves"] = movesArray;

    QJsonDocument doc(queryRoot);
    return doc.toJson(QJsonDocument::Compact);
}

void AiHandler::newFenReceived(QString fen){
    m_fenList.append(fen);
}

//Gets the movements list
void AiHandler::uciMovesReceived(const QStringList uciList){
    m_uciMoves = uciList;
}

void AiHandler::newMoveReceived(QString move){
    m_movesList.append(move);
}

void AiHandler::initializeAI(){
    m_stockfishEvaluationsList.clear();
    m_movesList.clear();
    m_fenList.clear();
    m_isGeminiBusy = false;
    m_uciMoves.clear();
}

void AiHandler::setUser(const QString &username, const QString &color){
    m_user = username;
    m_userColor = color;
}

void AiHandler::setPersona(const int personaIndex){
    switch (personaIndex) {
    case 0:
        m_persona = PERSONA_FRIENDLY_BUDDY;
        qDebug() << "Persona updated to: Friendly Buddy";
        break;
    case 1:
        m_persona = PERSONA_STRICT_COACH;
        qDebug() << "Persona updated to: Strict GM Coach";
        break;
    case 2:
        m_persona = PERSONA_TRASH_TALKER_ROLE;
        qDebug() << "Persona updated to: Trash-Talking Rival";
        break;
    default:
        qWarning() << "Invalid persona index received:" << personaIndex << ". Falling back to Friendly Buddy.";
        m_persona = PERSONA_FRIENDLY_BUDDY;
        break;
    }
}


//this function will be uysed later
// void AiHandler::explainNextMove(){
// if(isIaReady && !m_evaluationsList.isEmpty() && !m_isIaBusy){
//     if(!m_stockfishEvaluationsList.isEmpty() && !m_fenList.isEmpty() && !m_movesList.isEmpty()){
//         int evaluation = m_stockfishEvaluationsList.deque;
//         QString fen = m_fenList.dequeue();
//         QString move = m_movesList.dequeue();

//         requestExplanation(fen,move,evaluation);
//         m_isGeminiBusy=true;
//         // analyzePosition("startpos", nextMove);
//     }
// }
