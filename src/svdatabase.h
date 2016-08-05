#ifndef SVDATABASE_H
#define SVDATABASE_H

#include <QHash>
#include "card.h"
#include <QString>
#include <QPixmap>
#include <QVector>

typedef unsigned long long int ulong64;

class svDatabase
{
public:
    svDatabase();
    void addCard(int id, Card card);
    Card getCard(int id);
    void updateCard(int id, Card card);
    const QPixmap *getPortrait(int id);
    const QPixmap *getCost(int cost);
    std::vector<ulong64> getCostPHash();
    int getCostfromPHash(ulong64);
    void load();
    void save();
    int size();
    QVector<int> cardID;
private:
    QHash<int , Card> cardMap;
    QHash<int , QPixmap> portraitMap;
    QHash<int , QPixmap> costMap;
    QHash<int , ulong64> costPHashMap;
};

#endif // SVDATABASE_H
