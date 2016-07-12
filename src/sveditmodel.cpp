#include "sveditmodel.h"


SVEditModel::SVEditModel(QObject *parent)
  : QAbstractListModel(parent)
{
}

int SVEditModel::rowCount(const QModelIndex &) const
{
   return cardsInDeck.size();
}

QVariant SVEditModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(index.row() >= cardsInDeck.size() || index.row() < 0)
        return QVariant();

    if(role == Qt::DisplayRole)
    {
        return cardsInDeck[index.row()];
    }
    return QVariant();
}

void SVEditModel::addCard(int ID)
{
    beginInsertRows(QModelIndex(),cardsInDeck.size(),cardsInDeck.size());
    cardsInDeck.push_back(ID);
    endInsertRows();
}

void SVEditModel::slotPlusRow(int row){
    if (playingDeck->getDeckSize() < 40)
    {
        int ID = cardsInDeck[row];
        playingDeck->addCard(ID);
        emit deckChanged(playingDeck->getDeckSize());
    }
}

void SVEditModel::sortList()
{
    for (int i = 1; i < cardsInDeck.size(); i++)
    {
        int j = i;

        while (j > 0 &&
               database->getCard(cardsInDeck[j-1]).manaCost >
               database->getCard(cardsInDeck[j]).manaCost)
        {
            int tempCard = cardsInDeck[j];
            cardsInDeck[j] = cardsInDeck[j-1];
            cardsInDeck[j-1] = tempCard;
            j--;
        }
    }
}
