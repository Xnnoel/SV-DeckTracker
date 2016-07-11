#ifndef CARDDELEGATE_H
#define CARDDELEGATE_H

#include "svdatabase.h"
#include "cardlist.h"
#include <QStyledItemDelegate>
#include <QPainter>
#include <QFont>
#include <QApplication>
#include <QPoint>

class CardDelegate : public QStyledItemDelegate
{   
    Q_OBJECT
public:
    CardDelegate(QObject *parent = 0);
    void setPointers(svDatabase *db, cardlist * cd);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option,
                        const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool editorEvent(QEvent *event, QAbstractItemModel*, const QStyleOptionViewItem &option, const QModelIndex &index);


    enum datarole {COST = Qt::UserRole + 100,ID = Qt::UserRole+101,COUNT = Qt::UserRole+102,NAME = Qt::UserRole+103};
    bool editMode = false;
private:
    svDatabase *database;
    cardlist* playingDeck;
    QPixmap up;
    QPixmap down;
    QPixmap minus;
signals:
    void upClicked(int row);
    void downClicked(int row);
    void minusClicked(int row);
};

#endif // CARDDELEGATE_H
