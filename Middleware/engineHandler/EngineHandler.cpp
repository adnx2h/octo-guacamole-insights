#include "EngineHandler.h"
#include <QRegularExpression>
#include <QDebug>

EngineHandler::EngineHandler(QObject *parent)
    : QObject{parent}
{}

bool EngineHandler::isValidPGN(const QString &pgn) {
    // PGN validation using regular expressions
    //  Check for White/Black tags, and result.

    bool valid = true;
    QString result;
    QString whiteTag;
    QString blackTag;

    // Check for the presence of the [White ...] and [Black ...] tags
    QRegularExpression whiteRegex("\\[White\\s+\".*\"\\]");
    QRegularExpression blackRegex("\\[Black\\s+\".*\"\\]");
    whiteTag = whiteRegex.match(pgn).captured(0);
    blackTag = blackRegex.match(pgn).captured(0);

    if (whiteTag.isEmpty() || blackTag.isEmpty())
    {
        qDebug() << "PGN failed - Missing White or Black Tag";
        valid = false;
    }

    // Check for the result at the end of the PGN string
    QRegularExpression resultRegex("(\\d-\\d|\\*|1-0|0-1|1/2-1/2)$");
    result = resultRegex.match(pgn).captured(0);
    if (result.isEmpty())
    {
        qDebug() << "PGN failed - Missing result";
        valid = false;
    }
    return valid;
}

// Function to parse a PGN chess game
EngineTypes::PgnData EngineHandler::parsePgn(const QString& pgnString) {
    EngineTypes::PgnData data;
    QStringList lines = pgnString.split('\n');
    QString moveText;
    if(!isValidPGN(pgnString)){
        qDebug()<<"PGN not Valid";
        return data;
    }

    qDebug() << "PGN Valid";
    QRegularExpression moveOnlyRegex("([^\\s.]+)(?=\\s+|$)");
    QRegularExpression resultRegex("(\\s*)(1-0|0-1|1/2-1/2|\\*)$");
    QRegularExpression tagRegex("\\[(\\w+)\\s+\"([^\"]*)\"\\]");

    QMap<QString, QString> tags;
    QList<QString> moves;


    for (const QString& line : lines) {
        QRegularExpressionMatch tagMatch = tagRegex.match(line);
        if (tagMatch.hasMatch()) {
            tags[tagMatch.captured(1)] = tagMatch.captured(2);
        } else if (!line.trimmed().isEmpty() && !line.startsWith(";")) {
            moveText += line + " ";
        }
    }

    // First, try to find the game result at the end and isolate the moves part
    QRegularExpressionMatch resultMatch = resultRegex.match(moveText);
    QString movesPart = moveText.trimmed(); // Trim to handle potential leading/trailing spaces
    if (resultMatch.hasMatch() && resultMatch.capturedEnd(1) == movesPart.length()) {
        movesPart = movesPart.left(resultMatch.capturedStart(1)).trimmed();
    }

    // Extract moves using the simplified regex
    QRegularExpressionMatchIterator moveIterator = moveOnlyRegex.globalMatch(movesPart);
    while (moveIterator.hasNext()) {
        QRegularExpressionMatch moveMatch = moveIterator.next();
        moves.append(moveMatch.captured(1).trimmed());
    }

    data.tags = tags;
    data.moves = moves;
    return data;
}
