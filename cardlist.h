#ifndef CARDLIST_H
#define CARDLIST_H

#include "card.h"
#include "perceptualhash.h"
#include "svdatabase.h"

#include <vector>
class cardlist
{
public:
    std::vector<int> cardsInDeck;   //has id of all cards in deck EASIER TO STORE
    std::vector<int> countInDeck;   //has the counter per card
    std::vector<ulong64> deckPHash; //updates as the same as the upper 2
    void addCard(int id);
    cardlist(std::string deckname, svDatabase* database);          //name of card
private:
    std::string deckName;
    svDatabase* databasePtr;

};

#endif // CARDLIST_H
