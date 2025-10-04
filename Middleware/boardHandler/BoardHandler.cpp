#include "BoardHandler.h"
#include <QRegularExpression>
#include <QDebug>

BoardHandler::BoardHandler(QObject *parent)
    : QObject{parent}
{
}

void BoardHandler::newEvaluation(int ev){
    m_movesEvaluations.append(ev);
}

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
    QStringList lines = pgnString.split('\n');
    QString moveText;
    QString result; // To store the game result
    QMap<QString, QString> tags;
    QList<QString> moves;

    // Initialize the board to the starting position
    initializeBoard();

    emit sgn_startEngine();

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
            QString san = moveMatch.captured(1).trimmed();
            moves.append(san);

            chess::Move mo = chess::uci::parseSan(tempBoard,san.toStdString());
            m_movesObject.moves.append(mo);
            tempBoard.makeMove(mo);
            toUciMove(mo);

            emit sgn_newFen(QString::fromStdString(tempBoard.getFen()));
        }

        m_board = tempBoard;
        m_board = chess::Board(); //reset the board
        emit piecePositionsChanged(); // Update QML to show the initial board

        pgn_data.tags = tags;
        pgn_data.moves = moves;
        pgn_data.result = result;

        emit rawMovesListReady(pgn_data.moves);
        // emit tagsReady(pgn_data.tags);
        // emit resultReady(pgn_data.result);
        emit setDefaultPosition();
        // emit sgn_startEngine();
        emit sgn_uciMovesReady(uciMovesList);
    }
    else{
        qDebug()<<"PGN not Valid";
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

void BoardHandler::prevMove(){
    if (m_moveIndex >= 0) {
        const chess::Move moveToUnmake = m_movesObject.moves.at(m_moveIndex);
        m_board.unmakeMove(moveToUnmake);
        m_moveIndex--;
        emit piecePositionsChanged();
        qDebug() << "Prev Move: Index:" << m_moveIndex;
        if(m_moveIndex >= 0){
            emit sgn_evalPositionsChanged(m_movesEvaluations.at(m_moveIndex));
            qDebug() <<"Eval: "<<m_movesEvaluations.at(m_moveIndex);
        }else{
            emit sgn_evalPositionsChanged(0);
            qDebug() <<"Eval: "<<0;
        }
    } else {
        qDebug() << "Already at the beginning of the game (initial board state).";
    }
}

void BoardHandler::nextMove(){
    if (m_moveIndex < (int)m_movesObject.moves.size() - 1) {
        m_moveIndex++;
        const chess::Move moveToMake = m_movesObject.moves.at(m_moveIndex);
        m_board.makeMove(moveToMake);
        emit piecePositionsChanged();
        emit sgn_evalPositionsChanged(m_movesEvaluations.at(m_moveIndex));

        qDebug() << "Next Move: Index:" << m_moveIndex << "Evaluation:" << m_movesEvaluations.at(m_moveIndex);
    } else {
        qDebug() << "No More Moves (already at the end of the game).";
    }
}

void BoardHandler::toUciMove(chess::Move move){
    chess::Square from = move.from();
    chess::Square to = move.to();

    std::string fromStr = static_cast<std::string>(from);
    std::string toStr = static_cast<std::string>(to);

    QString uciMove = QString::fromStdString(fromStr + toStr);
    uciMovesList.append(uciMove);
    emit sgn_newUCIMove(uciMove);
}

////here starts the chess proxy
QVariantList BoardHandler::piecePositions() const
{
    return generatePiecePositions();
}

void BoardHandler::initializeBoard()
{
    m_board= chess::Board(chess::constants::STARTPOS,false); //initializes the board to the starting position
    tempBoard = chess::Board(chess::constants::STARTPOS,false);
    m_moveIndex = -1;
    m_movesObject.moves.clear();
    m_movesEvaluations.clear();
    emit piecePositionsChanged(); // Notify QML that the data has changed
}

QString BoardHandler::pieceToString(chess::Piece piece) const
{
    if (piece == chess::Piece::NONE) {
        return QString(); // Return empty string for no piece
    }

    // Determine color character ('w' for white, 'b' for black)
    QString colorChar = (piece.color() == chess::Color::WHITE) ? "w" : "b";
    QString typeChar;

    // Determine piece type character (P, N, B, R, Q, K)
    switch (static_cast<std::uint8_t>(piece.type())) {
    case static_cast<std::uint8_t>(chess::PieceType::PAWN):   typeChar = "P"; break;
    case static_cast<std::uint8_t>(chess::PieceType::KNIGHT): typeChar = "N"; break;
    case static_cast<std::uint8_t>(chess::PieceType::BISHOP): typeChar = "B"; break;
    case static_cast<std::uint8_t>(chess::PieceType::ROOK):   typeChar = "R"; break;
    case static_cast<std::uint8_t>(chess::PieceType::QUEEN):  typeChar = "Q"; break;
    case static_cast<std::uint8_t>(chess::PieceType::KING):   typeChar = "K"; break;
    case static_cast<std::uint8_t>(chess::PieceType::NONE):
        // This case should ideally be caught by the initial 'if (piece == chess::Piece::NONE)'
        // but is included for completeness and robustness.
        return QString();
    }
    return colorChar + typeChar; // Combine to "wP", "bR", etc.
}

QVariantList BoardHandler::generatePiecePositions() const
{
    QVariantList positions;
    // Iterate through all 64 squares (0-63)
    for (int i = 0; i < 64; ++i) {
        chess::Square sq(i); // Create a chess::Square from the index
        chess::Piece piece = m_board.at(sq); // Get the piece at the current square

        if (piece != chess::Piece::NONE) { // If there is a piece on this square
            QVariantMap pieceMap;
            int file = i % 8;
            int rank = i / 8;
            int qmlIndex = (7 - rank) * 8 + file; //maps a8 index 0, a1 index 56, h8 index 7, h1 index 63
            pieceMap["index"] = qmlIndex;
            pieceMap["piece"] = pieceToString(piece); // Convert to QML-friendly string
            positions.append(pieceMap);
        }
    }
    return positions;
}
