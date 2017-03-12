#include "svdatabase.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "QFile"
#include "QDir"

#include "QTextStream"

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

            addCard(card.ID, card);
        }
    // Sort list here
    qSort(cardID);

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
        card["Name"] = tempcard.name;

        cardArray.append(card);
    }
    gameObject["Cards"] = cardArray;

    QJsonDocument saveDoc(gameObject);
    saveFile.write(saveDoc.toJson());
    saveFile.close();

}
