#ifndef BOARDHANDLER_H
#define BOARDHANDLER_H

#include <QObject>
#include <QMap>
#include "BoardTypes.h"
#include "QQueue"

#include <QVariantList> // To hold a list of QVariantMap for pieces
#include <QVariantMap>  // To hold piece data (index, piece string)
#include "chess.hpp"    // Include the chess library

class BoardHandler : public QObject
{
    Q_OBJECT
    // Declare a Q_PROPERTY that QML can read.
    // It's a list of maps, where each map contains "index" (int) and "piece" (QString).
    Q_PROPERTY(QVariantList piecePositions READ piecePositions NOTIFY piecePositionsChanged)


public:
    explicit BoardHandler(QObject *parent = nullptr);
    Q_INVOKABLE void parsePgn(const QString& pgnString);
    Q_INVOKABLE void setFEN(QString& fen);
    Q_INVOKABLE void getFEN();
    Q_INVOKABLE void previousMove();
    Q_INVOKABLE void nextMove();

    /**
     * @brief Getter for the piecePositions Q_PROPERTY.
     * @return A QVariantList representing the current state of the board.
     */
    QVariantList piecePositions() const;

    /**
     * @brief Resets the chess board to its initial starting position.
     * This method is Q_INVOKABLE so it can be called from QML.
     */
    Q_INVOKABLE void resetBoard();

    // You can add more Q_INVOKABLE methods here for board interaction from QML,
    // e.g., making moves. Example commented out below:
    // Q_INVOKABLE void makeMove(int fromIndex, int toIndex, const QString& promotionPiece = "");
private:
    bool isValidPGN(const QString &pgn);
    chess::Board m_board; // The underlying chess board object from chess.hpp
    /**
     * @brief Helper function to convert the internal chess::Board state to a QML-friendly QVariantList.
     * @return A QVariantList of QVariantMaps, each containing "index" and "piece" (e.g., "wR", "bP").
     */
    QVariantList generatePiecePositions() const;

    /**
     * @brief Helper function to convert a chess::Piece object to its QML string representation.
     * @param piece The chess::Piece object.
     * @return A QString like "wR" for white rook, "bP" for black pawn, or empty if no piece.
     */
    QString pieceToString(chess::Piece piece) const;
    int moveIndex;
    BoardTypes::PgnData pgn_data;
    chess::Board tempBoard;
    struct MovesObject {
        QVector<chess::Move> moves;
    } movesObject;

signals:
    void rawMovesListReady(const QStringList& moves);
    void fenReady(const QString& fen);
    void setDefaultPosition();
    void piecePositionsChanged();
};

#endif // BOARDHANDLER_H
