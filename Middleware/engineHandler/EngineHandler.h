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

    // // Structure to hold the parsed PGN data
    // struct PgnData {
    //     QMap<QString, QString> tags;
    //     QStringList moves;
    //     QString result;
    // };
    Q_INVOKABLE EngineTypes::PgnData parsePgn(const QString& pgnString);

signals:
};

#endif // ENGINEHANDLER_H
