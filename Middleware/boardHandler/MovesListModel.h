#ifndef MOVESLISTMODEL_H
#define MOVESLISTMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QStringList>
#include <QDebug>
#include "BoardTypes.h"

class MovesListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum MoveRoles {
        MoveNumberRole = Qt::UserRole + 1,
        WhiteMoveRole,
        BlackMoveRole,
        // Add a role to return the entire MoveItem object
        MoveItemObjectRole
    };

    explicit MovesListModel(QObject *parent = nullptr);

    // QAbstractListModel overrides
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void processMoves(const QStringList& rawMoves);

private:
    QList<BoardTypes::MoveItem*> m_moveItems;
};

#endif // MOVESLISTMODEL_H
