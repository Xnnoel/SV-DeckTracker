#include "svlistmodel.h"
#include "carddelegate.h"

SVListModel::SVListModel(QObject *parent)
  : QAbstractListModel(parent)
{
}

int SVListModel::rowCount(const QModelIndex & /*parent*/) const
{
   return cardsInDeck.size();
}

QVariant SVListModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(index.row() >= cardsInDeck.size() || index.row() < 0)
        return QVariant();

    if(role == Qt::DisplayRole)
    {
        QStringList item;
        item.push_back(QString::number(cardsInDeck[index.row()].first));
        item.push_back(QString::number(cardsInDeck[index.row()].second));

        return QVariant(item);

    }
    return QVariant();
}

void SVListModel::addCard(int id)
{
    for (int i = 0; i < cardsInDeck.size(); i++)
    {
        if (cardsInDeck[i].first == id)
        {
            return;
        }
    }

    //if card doesn't exist, create new card
    beginInsertRows(QModelIndex(),cardsInDeck.size(),cardsInDeck.size());
    cardsInDeck.push_back(QPair<int,int>(id,1));
    endInsertRows();
}

void SVListModel::removeCard(int id)
{
    int row;
    std::vector<QPair<int,int>>::iterator iter = cardsInDeck.begin();
    for (int i = 0; i < cardsInDeck.size(); i++)
    {
        if (cardsInDeck[i].first == id)
        {
            row = i;
            break;
        }
        iter++;
    }

    beginRemoveRows(QModelIndex(), row, row );
    cardsInDeck.erase(iter);
    endRemoveRows();
}

void SVListModel::subCard(int id)
{
    int row;
    for (int i = 0; i < cardsInDeck.size(); i++)
    {
        if (cardsInDeck[i].first == id)
        {
            row = i;
            break;
        }
    }

    cardsInDeck[row].second--;
    QModelIndex cardIndex =  this->index(row,2);

    emit dataChanged(cardIndex, cardIndex);
    emit countChanged(getDeckSize());
}

int SVListModel::getDeckSize()
{
    int decksize = 0;
    for (int i = 0; i < cardsInDeck.size(); i++)
    {
        decksize += cardsInDeck[i].second;
    }

    return decksize;
}

bool SVListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole && !(index.row() >= cardsInDeck.size() || index.row() < 0))
    {
        int row = index.row();

        switch(index.column())
        {
        case 1:
            cardsInDeck[row].first = value.toInt();
            break;
        case 2:
            cardsInDeck[row].second = value.toInt();
            break;
        default:
            return false;
        }
        emit dataChanged(index, index);
        emit countChanged(getDeckSize());
        return true;
    }
    return false;
}

void SVListModel::setCount(int id, int count)
{
    int row;
    for (int i = 0; i < cardsInDeck.size(); i++)
    {
        if (cardsInDeck[i].first == id)
        {
            row = i;
            break;
        }
    }
    cardsInDeck[row].second = count;
    QModelIndex cardIndex =  index(row,0);

    emit dataChanged(cardIndex, cardIndex);
    emit countChanged(getDeckSize());
}

void SVListModel::clearData()
{
    cardsInDeck.clear();
}

void SVListModel::slotUp(int row)
{
    if (cardsInDeck[row].second < playingDeck->countInDeck[row])
    {
        cardsInDeck[row].second++;
        QModelIndex cardIndex = index(row,0);
        emit dataChanged(cardIndex, cardIndex);
        emit countChanged(getDeckSize());
    }
}

void SVListModel::slotDown(int row)
{
    if (cardsInDeck[row].second > 0)
    {
        cardsInDeck[row].second--;
        QModelIndex cardIndex = index(row,0);
        emit dataChanged(cardIndex, cardIndex);
        emit countChanged(getDeckSize());
    }
}


int SVListModel::getCount(int id)
{
    int count = 0;

    for (int i = 0; i < cardsInDeck.size(); i++)
    {
        if (cardsInDeck[i].first == id)
        {
            count = cardsInDeck[i].second;
            break;
        }
    }
    return count;
}


void SVListModel::slotMinusRow(int row)
{
    int ID = cardsInDeck[row].first;
    playingDeck->removeCard(ID);
    if (cardsInDeck[row].second > 1)
    {
        cardsInDeck[row].second--;
        QModelIndex cardIndex = index(row,0);

        emit dataChanged(cardIndex, cardIndex);
    }
    else
    {
        removeCard(ID);
    }
    emit countChanged(getDeckSize());
}

