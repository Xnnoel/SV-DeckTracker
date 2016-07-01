#ifndef CARDLIST_H
#define CARDLIST_H

#include "card.h"
#include <vector>
#include "perceptualhash.h"

class cardlist
{
public:
    std::vector<Card> cardsInDeck;
    std::vector<ulong64> deckPHash;
    void addCard(std::string name, std::string filename, ulong64 pHash);
    cardlist(std::string);
private:
    std::string deckName;

};

#endif // CARDLIST_H
