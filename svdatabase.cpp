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

            card.ID = cardObject["ID"].toString().toInt();
            card.manaCost = cardObject["Cost"].toInt();
            card.name = cardObject["Name"].toString();

            QString pHashString = cardObject["pHash"].toString();
            card.pHash = pHashString.toDouble();

            addCard(card.ID, card);
        }


    // Save all cost icon
    for (int i = 1; i < 21; i++)
    {
        QPixmap image( dir.absolutePath() + "/data/Cost/cost_" + QString::number(i) + ".png");
        costMap.insert(i, image);
    }

    //save all phash values
    QJsonArray costs = database["Costs"].toArray();
    for (int costIndex = 0; costIndex < costs.size(); costIndex++)
    {
        QJsonObject costObject = costs[costIndex].toObject();
        int cost = costObject["number"].toInt();

        QString pHashString = costObject["pHash"].toString();
        ulong64 cardpHash = pHashString.toDouble();

        costPHashMap.insert(cost,cardpHash) ;
    }

    loadFile.close();
}
