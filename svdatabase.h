#ifndef SVDATABASE_H
#define SVDATABASE_H

#include <QHash>
#include "card.h"
#include <QString>
#include <QPixmap>

class svDatabase
{
public:
    svDatabase();
    void addCard(int id, Card card);
    Card getCard(int id);
    const QPixmap *getPortrait(int id);
    const QPixmap *getCost(int cost);
    void load();
    int size();
private:
    QHash<int , Card> cardMap;
    QHash<int , QPixmap> portraitMap;
    QHash<int , QPixmap> costMap;
};

#endif // SVDATABASE_H
