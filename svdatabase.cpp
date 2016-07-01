#include "svdatabase.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "QFile"

svDatabase::svDatabase()
{
}

void svDatabase::addCard(int id, Card card){
    cardMap.insert(id,card);
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
    QJsonArray cards = database["cards"].toArray();
    for (int cardIndex = 0; cardIndex < cards.size(); ++cardIndex) {
            Card card;
            QJsonObject cardObject = cards[cardIndex].toObject();
            card.ID = cardObject["ID"].toString();
            card.manaCost = cardObject["Cost"].toInt();
            card.name = cardObject["Name"].toString();
            QString pHashString = cardObject["pHash"].toString();
            int upperHalf = pHashString.left(9);
            ulong64 cardPHash= wordFileList[1].toInt();
            cardPHash << 32;
            cardPHash += wordFileList[2].toInt();

            level.read(levelObject);
            mLevels.append(level);
        }

    loadFile.close();

}
