#include "svdatabase.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "QFile"
#include "QDir"

#include "perceptualhash.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

svDatabase::svDatabase()
{
    load();
}

void svDatabase::addCard(int id, Card card){
    cardMap.insert(id,card);

    QDir dir(".");
    QPixmap image( dir.absolutePath() + "/data/Portraits/" + QString::number(id) + ".jpg");
    portraitMap.insert(id, image);
    cardID.push_back(id);
}

Card svDatabase::getCard(int id)
{
    return cardMap.value(id);
}

void svDatabase::updateCard(int id, Card card)
{
    cardMap.insert(id, card);
}

const QPixmap * svDatabase::getPortrait(int id)
{
    return &portraitMap.value(id);
}

const QPixmap * svDatabase::getCost(int cost)
{
    return &costMap.value(cost);
}

int svDatabase::size()
{
    return cardMap.size();
}

std::vector<ulong64> svDatabase::getCostPHash()
{
    std::vector <ulong64> v;

    // populate map somehow

    for( QHash<int , ulong64>::iterator it = costPHashMap.begin(); it != costPHashMap.end(); ++it ) {
        v.push_back( it.value() );
    }
    return v;
}

int svDatabase::getCostfromPHash(ulong64 val)
{
    return costPHashMap.key(val);
}

void svDatabase::load(){
    // load in files
    QDir dir(".");
    QString filename= dir.absolutePath() + "/data/database.json";

    QFile loadFile(filename);

    if (!loadFile.open(QIODevice::ReadOnly)) {
           qWarning("Couldn't open database.json");
    }

    QByteArray data = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(data));
    QJsonObject database = loadDoc.object();
    QJsonArray cards = database["Cards"].toArray();
    for (int cardIndex = 0; cardIndex < cards.size(); cardIndex++) {
            Card card;
            QJsonObject cardObject = cards[cardIndex].toObject();

            card.ID = cardObject["ID"].toInt();
            card.manaCost = cardObject["Cost"].toInt();
            card.name = cardObject["Name"].toString();
            card.pHash = cardObject["pHash"].toString().toDouble();
            card.newpHash = cardObject["newpHash"].toString().toDouble();

            addCard(card.ID, card);
        }
    // Sort list here
    qSort(cardID);

    // load all cost icon
    for (int i = 1; i < 21; i++)
    {
        QPixmap image( dir.absolutePath() + "/data/Cost/cost_" + QString::number(i) + ".png");
        costMap.insert(i, image);
    }

    // load all phash values
    QJsonArray costs = database["Costs"].toArray();
    for (int costIndex = 0; costIndex < costs.size(); costIndex++)
    {
        QJsonObject costObject = costs[costIndex].toObject();
        int cost = costObject["number"].toInt();
        ulong64 cardpHash = costObject["pHash"].toString().toDouble();

        costPHashMap.insert(cost,cardpHash) ;
    }

    loadFile.close();
}

void svDatabase::save()
{
    // Saves database into json
    QDir dir(".");
    QString filename= dir.absolutePath() + "/data/database.json";

    //Serialize into JSON
    QFile saveFile(filename);
    if (!saveFile.open(QIODevice::WriteOnly)) {
       qWarning("Couldn't open save file.");
   }
    QJsonObject gameObject;
    //fill stuff in here
    QJsonArray cardArray;

    QHash<int, Card>::iterator i;
    for (i = cardMap.begin(); i != cardMap.end(); ++i)
    {
        QJsonObject card;
        int id = i.key();
        Card tempcard = i.value();

        card["ID"] = id;
        card["Cost"] = tempcard.manaCost;
        card["pHash"] = QString::number((double)tempcard.pHash,'g',64);
        card["newpHash"] = QString::number((double)tempcard.newpHash,'g',64);
        card["Name"] = tempcard.name;

        cardArray.append(card);
    }
    gameObject["Cards"] = cardArray;


    QJsonArray numberHash;

    for (int j = 1; j < 11; ++j)
    {
        QJsonObject card;

        card["pHash"] = QString::number((double)costPHashMap.value(j),'g',64);
        card["number"] = j;

        numberHash.append(card);
    }
    QJsonObject card;

    card["pHash"] = QString::number((double)costPHashMap.value(18),'g',64);
    card["number"] = 18;

    numberHash.append(card);

    gameObject["Costs"] = numberHash;


    QJsonDocument saveDoc(gameObject);
    saveFile.write(saveDoc.toJson());
    saveFile.close();

}
