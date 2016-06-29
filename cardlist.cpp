#include "cardlist.h"

cardlist::cardlist(std::string filename):
    deckName(filename)
{
}

void cardlist::addCard(std::string name, std::string filename, ulong64 pHash)
{
    for (int i = 0; i < cardsInDeck.size(); i++)
    {
        Card* temp = &cardsInDeck[i];
        if (temp->filename == filename)
        {
            temp->amountInDeck++;
            return;
        }
    }

    //if card doesn't exist, create new card
    Card newCard;
    newCard.name = name;
    newCard.filename = filename;
    newCard.amountInDeck = 1;
    newCard.pHash = pHash;

    cardsInDeck.push_back(newCard);
    deckPHash.push_back(pHash);
}
