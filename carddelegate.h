#ifndef CARDDELEGATE_H
#define CARDDELEGATE_H

#include "svdatabase.h"
#include <QStyledItemDelegate>
#include <QPainter>
#include <QFont>
#include <QApplication>
#include <QPoint>

class CardDelegate : public QStyledItemDelegate
{
public:
    CardDelegate(svDatabase *db):database(db){};
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;
    enum datarole {Cost = Qt::UserRole + 100,ID = Qt::UserRole+101,Amount = Qt::UserRole+102,Name = Qt::UserRole+103};
private:
    svDatabase *database;
};

#endif // CARDDELEGATE_H
