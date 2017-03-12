#include "cardlist.h"

cardlist::cardlist(svDatabase* database):
    databasePtr(database), myClass(0), filename("")
{
}

double cardlist::makeDeckHash()
{
    double retVal = 0;
    for (int i = 0; i < cardsInDeck.size(); i++)
        retVal += cardsInDeck[i] + countInDeck[i];
    return retVal;
}

bool cardlist::cardExists(int id)
{
    for (int i = 0; i <cardsInDeck.size(); i++)
    {
        if (cardsInDeck[i] == id)
            return true;
    }
    return false;
}

void cardlist::addCard(int id)
{
    Card card = databasePtr->getCard(id);

    for (int i = 0; i < cardsInDeck.size(); i++)
    {
        if (cardsInDeck[i] == id)
        {
            if (countInDeck[i] < 3)
                countInDeck[i]++;
            return;
        }
    }

    //if card doesn't exist, create new card
    cardsInDeck.push_back(id);
    countInDeck.push_back(1);
}

void cardlist::removeCard(int id)
{
    for (int i = 0; i < cardsInDeck.size(); i++)
    {
        if (cardsInDeck[i] == id)
        {

            if (countInDeck[i] > 0)
                countInDeck[i]--;
            if (countInDeck[i] == 0){
                countInDeck.erase(countInDeck.begin() + i);
                cardsInDeck.erase(cardsInDeck.begin() + i);
            }
            return;
        }
    }
}

int cardlist::getPosition(int id){
    int index;
    for (int i = 0; i < cardsInDeck.size(); i++)
    {
        if (cardsInDeck[i] == id)
        {
            index = i;
            break;
        }
    }
    return index;
}

int cardlist::getDeckSize()
{
    int decksize = 0;
    for (int i = 0; i < countInDeck.size(); i++)
    {
        decksize += countInDeck[i];
    }
    return decksize;
}
