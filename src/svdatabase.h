#ifndef SVDATABASE_H
#define SVDATABASE_H

#include <QHash>
#include "card.h"
#include <QString>
#include <QPixmap>
#include <QVector>

class svDatabase
{
public:
    svDatabase();
    void addCard(int id, Card card);
    Card getCard(int id);
    void updateCard(int id, Card card);
    const QPixmap *getPortrait(int id);
    const QPixmap *getCost(int cost);
    void load();
    void save();
    int size();
    QVector<int> cardID;
private:
    QHash<int , Card> cardMap;
    QHash<int , QPixmap> portraitMap;
    QHash<int , QPixmap> costMap;
};

#endif // SVDATABASE_H
