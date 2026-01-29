#include "AiHandler.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>

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

void AiHandler::requestGameExplanation(const QString& gameAnalysisJson)
{
    if (gameAnalysisJson.isEmpty()) {
        qWarning() << "Cannot request game explanation: Input JSON string is empty.";
        return;
    }

    emit explanationRequestStatus(true); // Indicate loading

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
    const QString apiKey = qEnvironmentVariable("geminiApiKey");

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

void AiHandler::onGeminiGameApiReply(QNetworkReply* reply)
{
    qDebug() << "onGeminiGameApiReply";
    emit explanationRequestStatus(false); // Indicate loading finished
    m_gameExplanations.clear();          // Clear previous analysis

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Gemini API Error (Game):" << reply->errorString();
        emit aiError("Game Explanation Error: " + reply->errorString());
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
        emit aiError("AI response error: Could not parse analysis JSON.");
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
    emit gameExplanationReady(m_gameExplanations);
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
        requestGameExplanation(jsonAnalysisGame);
    }
    else{
        qWarning() << "Error: Data lists are not synchronized or are empty. Cannot create JSON query.";
    }
}

QString AiHandler::createGameJsonQuery(){
    int moveCount = m_uciMoves.size();

    QJsonArray movesArray;
    // ... (Iterate and Build the JSON Array 'movesArray')
    for (int i = 0; i < moveCount; ++i)
    {
        // ... (Build individual moveObject) ...
        QJsonObject moveObject;
        moveObject["move_number"] = i + 1;
        moveObject["color"] = (i % 2 == 0) ? "White" : "Black";
        // moveObject["san_move"] = m_movesList.at(i);
        moveObject["uci_move"] = m_uciMoves.at(i);
        moveObject["fen_before_move"] = m_fenList.at(i);

        int rawEval = m_stockfishEvaluationsList.at(i);

        // Convert integer evaluation to the required API string format (e.g., "+0.50" or "#5")
        // QString apiEvalString; this is not ok
        // if (rawEval > 10000) {
        //     apiEvalString = QString("#%1").arg(rawEval / 10000);
        // } else if (rawEval < -10000) {
        //     apiEvalString = QString("-#%1").arg(qAbs(rawEval / 10000));
        // } else {
        //     apiEvalString = QString("%1%2.%3").arg(rawEval >= 0 ? "+" : "").arg(rawEval / 100).arg(qAbs(rawEval % 100), 2, 10, QChar('0'));
        // }

        // Send both the raw value and the formatted string the API needs
        moveObject["evaluation_cp"] = rawEval;
        // moveObject["evaluation_api_format"] = apiEvalString; // Key for the API

        movesArray.append(moveObject);
    }

    // --- THIS IS WHERE TO SET THE PROMPT TEXT ---
    const QString promptText =
        "You are a world-class Chess Commentator and Grandmaster-level Analyst. Your task is to provide engaging, clear, and insightful natural language commentary for a sequence of moves."
        "\n\n**Instructions:**"
        "\n1.  **Strictly adhere to the provided Output Format.** Do not add extra introductory or concluding remarks outside of the main analysis section."
        "\n2.  Analyze each move in the provided list."
        "\n3.  Use the move's **Evaluation Change** and the context of the **Current FEN** to generate the commentary."
        "\n    * **Good Move (Evaluation improves for the moving side, or maintains a large advantage):** Explain the plan, the key strategic/tactical idea, or why the move is superior to other options."
        "\n    * **Inaccuracy/Mistake (Evaluation worsens for the moving side):** Identify the drawback of the move, what the player missed, and what the superior alternative was (if a clear one exists)."
        "\n    * **Normal/Forcing Move:** Briefly summarize the move's purpose and its positional impact."
        "\n4.  The output should be a single, flowing paragraph of commentary for the sequence of moves, using **Standard Algebraic Notation (SAN)** for the moves in your commentary."
        "\n\n**Input Data Format:**"
        "\n* **Starting FEN:** The position *before* the first move in the list is played."
        "\n* **Move List:** A sequential list of moves, each with its engine evaluation after the move. (Note: Use the FEN and Evaluation data provided in the JSON 'moves' array)."
       // "\n* **Output Format:** A single JSON block with the following structure: \n```json\n{\n  \"analysis_commentary\": [\n    {\n      \"move_number\": 1,\n      \"color\": \"White\",\n      \"explanation\": \"...\" \n    },\n    {\n      \"move_number\": 2,\n      \"color\": \"Black\",\n      \"explanation\": \"...\" \n    }\n    // ... and so on\n  ]\n}\n```"
                               "\n* **Output Format:** A single JSON block with the following structure: \n```json\n{\n  \"overall_commentary\": \"[YOUR OVERALL SUMMARY HERE]\",\n  \"analysis_commentary\": [\n    {\n      \"move_number\": 1,\n      \"color\": \"White\",\n      \"explanation\": \"...\" \n    },\n    {\n      \"move_number\": 2,\n      \"color\": \"Black\",\n      \"explanation\": \"...\" \n    }\n    // ... and so on\n  ]\n}\n```";
        //"\n* **Output Format:** A single JSON block with the following structure: \n```json\n{\n \"analysis_commentary\": \"[YOUR COMPLETE PARAGRAPH OF COMMENTARY HERE]\"\n}\n```";
    qDebug()<<promptText;

    // 3. Create the Final JSON Document
    QJsonObject queryRoot;
    queryRoot["analysis_request_type"] = "commentary";
    queryRoot["prompt_instructions"] = promptText;
    queryRoot["starting_fen"] = m_fenList.first(); // Use the first FEN as the starting position for the prompt
    queryRoot["total_moves"] = moveCount;
    queryRoot["moves"] = movesArray;

    QJsonDocument doc(queryRoot);
    qDebug()<<doc.toJson(QJsonDocument::Compact);



    QByteArray jsonBytes = doc.toJson(QJsonDocument::Compact);
    QFile file("analysis_query.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << QString::fromUtf8(jsonBytes);
        file.close();
        qDebug() << "Full JSON written to analysis_query.json";
    }





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
