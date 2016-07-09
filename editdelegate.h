#ifndef EDITDELEGATE_H
#define EDITDELEGATE_H

#include <QStyledItemDelegate>
#include "svdatabase.h"
#include "cardlist.h"

class editDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    editDelegate(QObject *parent = 0);
    void setPointers(svDatabase *db, cardlist * cd);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option,
                        const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool editorEvent(QEvent *event, QAbstractItemModel*, const QStyleOptionViewItem &option, const QModelIndex &index);
private:
    svDatabase* database;
    cardlist* playingDeck;
    QPixmap plus;
    QPixmap minus;
signals:
    void plusClicked(int row);
    void minusClicked(int row);
};

#endif // EDITDELEGATE_H
