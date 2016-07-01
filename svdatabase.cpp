#include "svdatabase.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "QFile"

svDatabase::svDatabase()
{
    load();
}

void svDatabase::addCard(int id, Card card){
    cardMap.insert(id,card);
}

Card svDatabase::getCard(int id)
{
    return cardMap.value(id);
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

}
