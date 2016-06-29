#ifndef CARDLIST_H
#define CARDLIST_H

#include <vector>
#include "perceptualhash.h"

class cardlist
{
public:
    struct Card{
        std::string name;       //card name to display
        std::string filename;   //filename
        int amountInDeck;
        ulong64 pHash;
    };
    std::vector<Card> cardsInDeck;
    std::vector<ulong64> deckPHash;
    void addCard(std::string name, std::string filename, ulong64 pHash);
    cardlist(std::string);
private:
    std::string deckName;

};

#endif // CARDLIST_H
