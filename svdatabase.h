#ifndef SVDATABASE_H
#define SVDATABASE_H

#include <QHash>
#include <card.h>
#include <QString>



class svDatabase
{
public:
    svDatabase();
    void addCard(int id, Card card);
    void load();
private:
    QHash<int , Card> cardMap;
};

#endif // SVDATABASE_H
