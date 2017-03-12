#ifndef CARDLIST_H
#define CARDLIST_H

#include "card.h"
#include "svdatabase.h"

#include <vector>
class cardlist
{
public:
    std::vector<int> cardsInDeck;   //has id of all cards in deck EASIER TO STORE
    std::vector<int> countInDeck;   //has the counter per card
    void addCard(int id);
    double makeDeckHash();
    void removeCard(int id);
    void clear(){deckName.clear(); cardsInDeck.clear(); countInDeck.clear();filename.clear();}
    int getPosition(int id);
    std::string getName(){return deckName;}
    void setName(std::string name){deckName = name;}
    cardlist(svDatabase* database);          //name of card
    bool getMode(){return editMode;}
    void setMode(bool mode){editMode = mode;}
    int getClass(){return myClass;}
    void setClass(int classnum){myClass = classnum;}
    int getDeckSize();
    QString getFileName(){return filename;}
    void setFileName(QString fileName){filename = fileName;}

private:
    std::string deckName;
    svDatabase* databasePtr;
    int myClass;        //deck class
    bool editMode;
    QString filename;   //file loaded from
};

#endif // CARDLIST_H
