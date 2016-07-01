#ifndef CARD_H
#define CARD_H
#include <QString>

typedef unsigned long long int  ulong64;


struct Card{
    QString name;       //card name to display
    int manaCost;
    int ID;
    ulong64 pHash;
};

#endif // CARD_H
