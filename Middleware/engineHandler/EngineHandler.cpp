#include "EngineHandler.h"
#include <QRegularExpression>
#include <QDebug>

EngineHandler::EngineHandler(QObject *parent)
    : QObject{parent}
{}

bool EngineHandler::isValidPGN(const QString &pgn) {
    // Regular expression for metadata tags: [Tag "Value"]
    QRegularExpression metadataRegex(
        "^\\[([A-Za-z]+)\\s+\"([^\"]*)\"\\]\\s*"
        );

    // Regular expression for move text
    QRegularExpression moveTextRegex(
        "([1-9][0-9]*\\.)?\\s*"                 // Move number (optional)
        "("
        "([O-O-O|O-O])|"                 // Castling
        "([KQRBN]?[a-h]?[1-8]?[-x]?[a-h][1-8][+#]?)" // Standard move
        "(=[QRBN])?"                   // Promotion (optional)
        ")"
        "(\\s*(1-0|0-1|1\\/2-1\\/2|\\*))?\\s*"  // Optional result
        );

    QStringList lines = pgn.split('\n', Qt::SkipEmptyParts); // Split into lines

    bool metadataSectionEnded = false; // Track metadata section
    QString moveText;

    for (const QString &line : lines) {
        if (!metadataSectionEnded) {
            // Check for metadata tags only if we haven't reached the move text
            if (!metadataRegex.match(line).hasMatch()) {
                // If it's not a metadata tag, assume it's the start of move text
                metadataSectionEnded = true;
                if (!metadataRegex.match(line).hasMatch())
                {
                    qDebug() << "Invalid metadata:" << line;
                    return false;
                }
            }
        }
        if (metadataSectionEnded) {
            moveText += line + " ";
        }
    }

    if(metadataSectionEnded){
        if (!moveTextRegex.match(moveText).hasMatch()) {
            qDebug() << "Invalid move text:" << moveText;
            return false; // Invalid move text
        }
    }
    return true;
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
