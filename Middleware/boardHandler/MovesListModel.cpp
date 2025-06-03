#include "MovesListModel.h"
#include <QDebug>
MovesListModel::MovesListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MovesListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_moveItems.count();
}

QVariant MovesListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_moveItems.size())
        return QVariant();

    EngineTypes::MoveItem* item = m_moveItems.at(index.row());
    if (!item) return QVariant();

    switch (role) {
    case MoveNumberRole: return item->moveNumber();      // This is correct (returns QString)
    case WhiteMoveRole:{
     // Add debugging within the data() method
     QString move_str = item->whiteMove();
     return move_str; // Explicitly return the QString    
    }
    case BlackMoveRole:{  
        return item->blackMove();}
    case MoveItemObjectRole: return QVariant::fromValue(item); // Returns the full MoveItem object
    default: return QVariant();
    }
}

QHash<int, QByteArray> MovesListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[MoveNumberRole] = "moveNumber";
    roles[WhiteMoveRole] = "whiteMove";
    roles[BlackMoveRole] = "blackMove";
    roles[MoveItemObjectRole] = "moveItemObject";
    return roles;
}

void MovesListModel::processMoves(const QStringList& rawMoves)
{
    beginResetModel();

    qDeleteAll(m_moveItems);
    m_moveItems.clear();

    for (int i = 0; i < rawMoves.count(); i += 2) {
        QString moveNumber = QString::number((i / 2) + 1);
        QString whiteMove = rawMoves.at(i);
        QString blackMove = (i + 1 < rawMoves.count()) ? rawMoves.at(i + 1) : "";

        m_moveItems.append(new EngineTypes::MoveItem(moveNumber, whiteMove, blackMove, this));
    }

    endResetModel();
}
