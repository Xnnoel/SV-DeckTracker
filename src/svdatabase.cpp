#include "svdatabase.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "QVariantMap"
#include "QFile"
#include "QDir"

#include "QTextStream"


svDatabase::svDatabase()
{
    load();
}

void svDatabase::addCard(int id, Card card){
    cardMap.insert(id,card);

    QDir dir(".");
    QString path = dir.absolutePath() + "/data/Portraits/" + QString::number(id) + ".jpg";

    QPixmap image( path);
    if (image.isNull())
        image.load(dir.absolutePath() + "/data/Portraits/NoImage.jpg");
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
    QJsonObject card1 = database["data"].toObject();

    QJsonArray cards = card1["cards"].toArray();

    for (int cardIndex = 0; cardIndex < cards.size(); cardIndex++) {
            Card card;
            QJsonObject cardObject = cards[cardIndex].toObject();

            card.clan = cardObject["clan"].toInt();
            card.ID = cardObject["card_id"].toInt();
            card.manaCost = cardObject["cost"].toInt();
            card.name = cardObject["card_name"].toString();

            addCard(card.ID, card);
        }
    // Sort list here
    qSort(cardID);

    loadFile.close();

    // load all cost icon
    for (int i = 1; i < 21; i++)
    {
        QPixmap image( dir.absolutePath() + "/data/Cost/cost_" + QString::number(i) + ".png");
        costMap.insert(i, image);
    }
}

void svDatabase::save()
{
    /*
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
*/
}
