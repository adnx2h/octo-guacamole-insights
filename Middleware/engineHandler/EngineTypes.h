#ifndef ENGINETYPES_H
#define ENGINETYPES_H

#include <QMap>
#include <QStringList>
#include <QString>
#include <QDebug>
#include <QObject>

// Structure to hold the parsed PGN data
namespace EngineTypes{

struct PgnData {
    QMap<QString, QString> tags;
    QStringList moves;
    QString result;
};

class MoveItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString moveNumber READ moveNumber WRITE setMoveNumber NOTIFY moveNumberChanged)
    Q_PROPERTY(QString whiteMove READ whiteMove WRITE setWhiteMove NOTIFY whiteMoveChanged)
    Q_PROPERTY(QString blackMove READ blackMove WRITE setBlackMove NOTIFY blackMoveChanged)

public:
    explicit MoveItem(const QString &moveNumber, const QString &whiteMove, const QString &blackMove, QObject *parent = nullptr)
        : QObject(parent), m_moveNumber(moveNumber), m_whiteMove(whiteMove), m_blackMove(blackMove) {}

    QString moveNumber() const { return m_moveNumber; }
    void setMoveNumber(const QString &moveNumber) {
        if (m_moveNumber != moveNumber) {
            m_moveNumber = moveNumber;
            emit moveNumberChanged();
        }
    }


    QString whiteMove() const {
        return m_whiteMove;
    }
    void setWhiteMove(const QString &whiteMove) {
        if (m_whiteMove != whiteMove) {
            m_whiteMove = whiteMove;
            emit whiteMoveChanged();
        }
    }


    QString blackMove() const {
        return m_blackMove;
    }
    void setBlackMove(const QString &blackMove) {
        if (m_blackMove != blackMove) {
            m_blackMove = blackMove;
            emit blackMoveChanged();
        }
    }

signals:
    void moveNumberChanged();
    void whiteMoveChanged();
    void blackMoveChanged();

private:
    QString m_moveNumber;
    QString m_whiteMove;
    QString m_blackMove;
};

} // namespace EngineTypes

#endif // ENGINETYPES_H
