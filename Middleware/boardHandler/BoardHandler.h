#ifndef BOARDHANDLER_H
#define BOARDHANDLER_H

#include <QObject>
#include <QMap>
#include "BoardTypes.h"
#include "QQueue"
#include <QStringList>

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
    Q_INVOKABLE void prevMove();
    Q_INVOKABLE void nextMove();
    Q_INVOKABLE int getCurrentMoveIndex();

    /**
     * @brief Getter for the piecePositions Q_PROPERTY.
     * @return A QVariantList representing the current state of the board.
     */
    QVariantList piecePositions() const;

    /**
     * @brief Resets the chess board to its initial starting position.
     * This method is Q_INVOKABLE so it can be called from QML.
     */
    Q_INVOKABLE void initializeBoard();
    void newEvaluation(int);

private:
    QStringList uciMovesList;
    QList<int> m_movesEvaluations;
    int m_moveIndex;
    void toUciMove(chess::Move);
    bool isValidPGN(const QString &pgn);
    BoardTypes::PgnData pgn_data;
    chess::Board tempBoard;
    chess::Board m_board;
    /**
     * @brief Helper function to convert the internal chess::Board state to a QML-friendly QVariantList.
     * @return A QVariantList of QVariantMaps, each containing "index" and "piece" (e.g., "wR", "bP").
     */
    QVariantList generatePiecePositions() const;

    QString pieceToString(chess::Piece piece) const;
    
    struct MovesObject {
        QVector<chess::Move> moves;
    } m_movesObject;

signals:
    void rawMovesListReady(const QStringList& moves);
    void fenReady(const QString& fen);
    void setDefaultPosition();
    void piecePositionsChanged();
    void sgn_startEngine();
    void sgn_analyzeNewMove(std::string);
    void sgn_uciMovesReady(QStringList);
    void sgn_evalPositionsChanged(int newEval);
    void sgn_newUCIMove(QString uciMove);
    void sgn_newFen(QString fen);
};

#endif // BOARDHANDLER_H
