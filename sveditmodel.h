#ifndef SVEDITMODEL_H
#define SVEDITMODEL_H

#include <QAbstractListModel>
#include "svdatabase.h"
#include "cardlist.h"
#include "svlistmodel.h"


class SVEditModel : public QAbstractListModel
{
    Q_OBJECT
public slots:
    void slotPlusRow(int);
public:
    SVEditModel(QObject *parent = 0);
    void setPointer(svDatabase * db, cardlist* cd, SVListModel* lm){database = db; playingDeck = cd;listmodel = lm;}

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE ;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    void addCard(int ID);
    void clearCards(){cardsInDeck.clear();}
private:
    std::vector<int> cardsInDeck;   // ID of cards, click to add so no need for count
    svDatabase* database;
    cardlist* playingDeck;
    SVListModel* listmodel;
signals:
    void deckChanged(int);
};

#endif // SVEDITMODEL_H
