#include "BoardHandler.h"
#include <QRegularExpression>
#include <QDebug>

BoardHandler::BoardHandler(QObject *parent)
    : QObject{parent}
{}

bool BoardHandler::isValidPGN(const QString &pgn) {
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
    QRegularExpression resultRegex1("(\\d-\\d|\\*|1-0|0-1|1/2-1/2)$");
    result = resultRegex1.match(pgn).captured(0);
    if (result.isEmpty())
    {
        qDebug() << "PGN failed - Missing result";
        valid = false;
    }
    return valid;
}

// Function to parse a PGN chess game
void BoardHandler::parsePgn(const QString& pgnString) {
    EngineTypes::PgnData pgn_data;
    QStringList lines = pgnString.split('\n');
    QString moveText;
    QString result; // To store the game result
    QMap<QString, QString> tags;
    QList<QString> moves;

    if(isValidPGN(pgnString)){
        qDebug() << "PGN Valid";
        QRegularExpression moveOnlyRegex("([^\\s.]+)(?=\\s+|$)");
        QRegularExpression tagRegex("\\[(\\w+)\\s+\"([^\"]*)\"\\]");
        QRegularExpression resultRegex("(\\s*)(1-0|0-1|1/2-1/2|\\*)$");



        // Process lines to extract tags and build moveText
        for (const QString& line : lines) {
            QRegularExpressionMatch tagMatch = tagRegex.match(line);
            if (tagMatch.hasMatch()) {
                tags[tagMatch.captured(1)] = tagMatch.captured(2);
            } else if (!line.trimmed().isEmpty() && !line.startsWith(";")) {
                moveText += line + " ";
            }
        }

        QString trimmedMoveText = moveText.trimmed();
        QRegularExpressionMatch resultMatch = resultRegex.match(trimmedMoveText);

        // Extract the result if found at the end of the move text
        if (resultMatch.hasMatch()) {
            result = resultMatch.captured(2); // Capture the actual result (group 2)
            // Trim the result (and any leading whitespace) from the end of the move text
            trimmedMoveText.chop(resultMatch.captured(0).length());
            trimmedMoveText = trimmedMoveText.trimmed(); // Re-trim after chopping
        }

        // Extract moves from the trimmed move text
        QRegularExpressionMatchIterator moveIterator = moveOnlyRegex.globalMatch(trimmedMoveText);
        while (moveIterator.hasNext()) {
            QRegularExpressionMatch moveMatch = moveIterator.next();
            moves.append(moveMatch.captured(1).trimmed());
        }

        pgn_data.tags = tags;
        pgn_data.moves = moves;
        pgn_data.result = result;

        emit rawMovesListReady(pgn_data.moves);
        // emit tagsReady(pgn_data.tags);
        // emit resultReady(pgn_data.result);
    }
    else{
        qDebug()<<"PGN not Valid";
        //what to do????
    }
}

void BoardHandler::setFEN(QString& fen) {
    // This function should set the FEN string to the chess engine.
    // For now, we will just print the FEN to debug output.
    qDebug() << "Setting FEN:" << fen;

    // Here you would typically set the FEN in your chess engine or game state.
    // For example:
    // chessEngine.setFEN(fen);
}


void BoardHandler::getFEN() {
    // This function should return the current FEN string in the chess board
    // For now, we will just emit a signal with a placeholder value.
    QString fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; // Placeholder FEN
    emit fenReady(fen);
}
