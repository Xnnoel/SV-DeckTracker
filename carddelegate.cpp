#include "carddelegate.h"
#include "card.h"

void CardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter,option,index);
    // Save painter info if needed
    painter->save();

    QFont font = QApplication::font();

    font.setBold(true);
    font.setPointSize(10);

    QRect headerRect = option.rect;
    QRect subheaderRect = option.rect;
    QRect iconRect = option.rect;

    iconRect.setRight(iconRect.left() + 40);
    headerRect.setLeft(iconRect.right());
    headerRect.setTop(iconRect.top());

    subheaderRect.setLeft(subheaderRect.right() - 20);

    headerRect.setRight(subheaderRect.left());

    int myid = qvariant_cast<int>(index.data(ID));
    int mycost = qvariant_cast<int>(index.data(Cost));

    /////// PAINTER HERE DRAW FUNCS ONLY
    QColor color = QColor(255,255,255);
    QPixmap image = *(database->getPortrait(myid));
    painter->drawPixmap(QPoint(iconRect.left(), iconRect.top()) , image );
    QPixmap costimage = *(database->getCost(mycost));
    costimage = costimage.scaled(20,20);
    painter->drawPixmap(QPoint(iconRect.left() + iconRect.width()/2 - costimage.width()/2, iconRect.top() + iconRect.height()/2-costimage.height()/2) , costimage);
    painter->setPen(color);
    painter->drawRect(option.rect);
    painter->setFont(font);
    painter->drawText(headerRect.left(), headerRect.top() + headerRect.height()/2 + font.pointSize()/2,qvariant_cast<QString>(index.data(Name)));
    painter->drawText(subheaderRect.left(), subheaderRect.top() + subheaderRect.height()/2 + font.pointSize()/2,qvariant_cast<QString>(index.data(Amount)));
    // Restore painter info
    painter->restore();
}

//alocate each item size in listview.
QSize CardDelegate::sizeHint(const QStyleOptionViewItem &  option ,
                              const QModelIndex & index) const
{
    QString myid = qvariant_cast<QString>(index.data(ID));
    QPixmap hi= *(database->getPortrait(myid.toInt()));
    return(QSize(option.rect.width(),hi.height()));

}
