#include "svdatabase.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "QFile"
#include "QDir"

svDatabase::svDatabase()
{
    load();
}

void svDatabase::addCard(int id, Card card){
    cardMap.insert(id,card);

    QDir dir(".");
    QPixmap image( dir.absolutePath() + "/Portraits/" + QString::number(id) + ".jpg");
    portraitMap.insert(id, image);
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

void svDatabase::load(){
    // load in files
    QString filename="database.json";

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
    loadFile.close();

    // Save all cost icon
    QDir dir(".");
    for (int i = 1; i < 21; i++)
    {
        QPixmap image( dir.absolutePath() + "/Cost/cost_" + QString::number(i) + ".png");
        costMap.insert(i, image);
    }
}
