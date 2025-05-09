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
        qDebug() << "isValidPGN failed for: " << pgn << "Missing White or Black Tag";
        valid = false;
    }

    // Check for the result at the end of the PGN string
    QRegularExpression resultRegex("(\\d-\\d|\\*|1-0|0-1|1/2-1/2)$");
    result = resultRegex.match(pgn).captured(0);
    if (result.isEmpty())
    {
        qDebug() << "isValidPGN failed for: " << pgn << "Missing result";
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
    // Regular expression to match PGN tags: [Tag "Value"]
    QRegularExpression tagRegex("\\[(\\w+)\\s+\"([^\"]*)\"\\]");

    // Regular expression to match move numbers and moves (e.g., 1. e4 c5, 1... Nf6)
    QRegularExpression moveRegex("(\\d+\\.)\\s*([\\w\\+\\#\\=\\-\\/]+)\\s*([\\w\\+\\#\\=\\-\\s\\/]*)|(\\d+\\.\\.\\.)\\s*([\\w\\+\\#\\=\\-\\s\\/]+)");

    // Regular expression to match the game result
    QRegularExpression resultRegex("(1-0|0-1|1/2-1/2|\\*)");

    for (const QString& line : lines) {
        QRegularExpressionMatch tagMatch = tagRegex.match(line);
        if (tagMatch.hasMatch()) {
            QString tag = tagMatch.captured(1);
            QString value = tagMatch.captured(2);
            data.tags[tag] = value;
        } else if (!line.trimmed().isEmpty() && !line.startsWith(";")) {
            // Append non-empty, non-comment lines to the move text
            moveText += line + " ";
        }
    }

    // Extract moves from the move text
    QRegularExpressionMatchIterator moveIterator = moveRegex.globalMatch(moveText);
    while (moveIterator.hasNext()) {
        QRegularExpressionMatch moveMatch = moveIterator.next();
        if (moveMatch.captured(2).isEmpty()) {
            // Black's move after move number
            data.moves.append(moveMatch.captured(5).trimmed());
        } else {
            // White's move
            data.moves.append(moveMatch.captured(2).trimmed());
            // Black's move (if present in the same line)
            if (!moveMatch.captured(3).trimmed().isEmpty()) {
                data.moves.append(moveMatch.captured(3).trimmed());
            }
        }
    }

    // Extract the result from the end of the move text
    QRegularExpressionMatch resultMatch = resultRegex.match(moveText);
    if (resultMatch.hasMatch()) {
        data.result = resultMatch.captured(1);
    } else {
        data.result = ""; // Or some other default value if no result is found
    }
    return data;
}
