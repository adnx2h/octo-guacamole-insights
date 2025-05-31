#ifndef ENGINEHANDLER_H
#define ENGINEHANDLER_H

#include <QObject>
#include <QMap>
#include "EngineTypes.h"

class EngineHandler : public QObject
{
    Q_OBJECT
private:
    bool isValidPGN(const QString &pgn);

public:
    explicit EngineHandler(QObject *parent = nullptr);
    Q_INVOKABLE void parsePgn(const QString& pgnString);
private:

signals:
    void rawMovesListReady(const QStringList& moves);
};

#endif // ENGINEHANDLER_H
