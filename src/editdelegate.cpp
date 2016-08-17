
#include "editdelegate.h"
#include <Qpainter>
#include <QFont>
#include <Qapplication>
#include <QMouseEvent>
#include <QDir>

editDelegate::editDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}


void editDelegate::setPointers(svDatabase *db, cardlist * cd){
    database = db;
    playingDeck = cd;

    QDir dir(".");
    plus = QPixmap(dir.absolutePath() +"/data/Markers/plus.png");
}

void editDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                    const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter,option,index);

    if (!index.isValid()){
        return;
    }
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
    subheaderRect.setLeft(subheaderRect.right() - 30);
    headerRect.setRight(subheaderRect.left());

    QRect plusRect;
    plusRect.setTop(option.rect.top() + 10);
    plusRect.setLeft(option.rect.left() + 225);
    plusRect.setRight(option.rect.left() + 240);
    plusRect.setBottom(option.rect.bottom() - 10);


    int myid = qvariant_cast<int>(index.data());

    Card card = database->getCard(myid);
    int mycost = card.manaCost;
    QString myname = card.name;

    /////// PAINTER HERE DRAW FUNCS ONLY
    //Draw background image of database
    QPixmap image = *(database->getPortrait(myid));
    painter->drawPixmap(QPoint(iconRect.left(), iconRect.top()) , image );
    QPixmap costimage = *(database->getCost(mycost));
    costimage = costimage.scaled(20,20);
    painter->drawPixmap(QPoint(iconRect.left() + iconRect.width()/2 - costimage.width()/2, iconRect.top() + iconRect.height()/2-costimage.height()/2) , costimage);

    //set color and draw font
    QColor color = QColor(255,255,255);
    painter->setPen(color);
    painter->setFont(font);
    painter->drawText(headerRect.left(), headerRect.top() + headerRect.height()/2 + font.pointSize()/2,myname);

    painter->restore();
}

QSize editDelegate::sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index) const
{
    int myid = qvariant_cast<int>(index.data());;

    QPixmap hi= *(database->getPortrait(myid));
    return(QSize(option.rect.width(),hi.height()));
}


bool editDelegate::editorEvent(QEvent *event, QAbstractItemModel*, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    // Emit a signal when the icon is clicked
    if(event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if(option.rect.contains(mouseEvent->pos()))
        {
            emit plusClicked(index.row());
        }
    }

    return false;
}
