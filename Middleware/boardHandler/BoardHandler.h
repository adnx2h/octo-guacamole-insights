#ifndef BOARDHANDLER_H
#define BOARDHANDLER_H

#include <QObject>
#include <QMap>
#include "EngineTypes.h"

class BoardHandler : public QObject
{
    Q_OBJECT
private:
    bool isValidPGN(const QString &pgn);

public:
    explicit BoardHandler(QObject *parent = nullptr);
    Q_INVOKABLE void parsePgn(const QString& pgnString);
    Q_INVOKABLE void setFEN(QString& fen);
    Q_INVOKABLE void getFEN();
private:

signals:
    void rawMovesListReady(const QStringList& moves);
    void fenReady(const QString& fen);
};

#endif // BOARDHANDLER_H
