#ifndef SVLISTMODEL_H
#define SVLISTMODEL_H

#include <QAbstractListModel>
#include <QStandardItem>
#include "svdatabase.h"
#include "cardlist.h"

class SVListModel : public QAbstractListModel
{
    Q_OBJECT
public slots:
    void slotUp(int row);
    void slotDown(int row);
    void slotMinusRow(int row);
public:
    SVListModel(QObject *parent = 0);
    void setPointer(svDatabase * db, cardlist* cd){database = db; playingDeck = cd;}

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE ;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    void blink(int row);
    void addCard(int id);
    void removeCard(int id);
    void subCard(int id);
    void setCount(int id, int count);
    void clearData();
    int getCount(int id);
private:
    std::vector<QPair<int,int>> cardsInDeck;   // ID of cards, vector pos = row pos
    cardlist* playingDeck;
    svDatabase * database;
    int getDeckSize();
signals:
    void countChanged(int);
    void deckChanged(int);
};

#endif // SVLISTMODEL_H
