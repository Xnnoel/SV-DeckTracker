#include "cardlist.h"

cardlist::cardlist(std::string filename, svDatabase* database):
    deckName(filename),
    databasePtr(database)
{
}

void cardlist::addCard(int id)
{
    Card card = databasePtr->getCard(id);

    for (int i = 0; i < cardsInDeck.size(); i++)
    {
        if (cardsInDeck[i] == id)
        {
            countInDeck[i]++;
            return;
        }
    }

    //if card doesn't exist, create new card
    cardsInDeck.push_back(id);
    countInDeck.push_back(1);
    deckPHash.push_back(card.pHash);
}
