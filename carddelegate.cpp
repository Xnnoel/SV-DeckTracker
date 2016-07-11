#include "carddelegate.h"
#include "card.h"
#include <QMouseEvent>



CardDelegate::CardDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void CardDelegate::setPointers(svDatabase *db, cardlist * cd)

{
    database = db;
    playingDeck = cd;
    up = QPixmap("up.png");
    down = QPixmap("down.png");
    minus = QPixmap("minus.png");
}

void CardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
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

    QRect upRect;
    upRect.setTop(option.rect.top() + 3);
    upRect.setLeft(option.rect.right() - 15);
    upRect.setRight(option.rect.right() - 2);
    upRect.setBottom(option.rect.top() + 16);

    QRect downRect;
    downRect.setTop(option.rect.top() + 19);
    downRect.setLeft(option.rect.right() - 15);
    downRect.setRight(option.rect.right() - 2);
    downRect.setBottom(option.rect.top() + 32);

    QRect minusRect;
    minusRect.setTop(option.rect.top() + 10);
    minusRect.setLeft(option.rect.left() + 240);
    minusRect.setRight(option.rect.left() + 255);
    minusRect.setBottom(option.rect.bottom() - 10);

    iconRect.setRight(iconRect.left() + 40);
    headerRect.setLeft(iconRect.right());

    subheaderRect.setLeft(subheaderRect.right() - 40);

    headerRect.setRight(subheaderRect.left());
    QStringList stringData = qvariant_cast<QStringList>(index.data());
    int myid = stringData[0].toInt();
    int mycount = stringData[1].toInt();

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
    if (mycount < playingDeck->countInDeck[index.row()])
        color = QColor(200,200,0);
    if (mycount == 0)
        color = QColor(220,0,0);
    painter->setPen(color);
    painter->setFont(font);
    painter->drawText(headerRect.left(), headerRect.top() + headerRect.height()/2 + font.pointSize()/2,myname);
    painter->drawText(subheaderRect.left(), subheaderRect.top() + subheaderRect.height()/2 + font.pointSize()/2, QString::number(mycount));

    //draw buttons for up and down maybe?
    if (!editMode)
    {
        painter->drawPixmap(QPoint(upRect.left(), upRect.top()) , up );
        painter->drawPixmap(QPoint(downRect.left(), downRect.top()) , down );
    }
    else
    {
        painter->drawPixmap(QPoint(minusRect.left(), minusRect.top()), minus);
    }
    // Restore painter info
    painter->restore();
}

//alocate each item size in listview.
QSize CardDelegate::sizeHint(const QStyleOptionViewItem &  option ,
                              const QModelIndex & index) const
{
    QStringList stringData = qvariant_cast<QStringList>(index.data());
    int myid = stringData[0].toInt();

    QPixmap hi= *(database->getPortrait(myid));
    return(QSize(option.rect.width(),hi.height()));

}

bool CardDelegate::editorEvent(QEvent *event, QAbstractItemModel*, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    // Emit a signal when the icon is clicked
    if (!editMode)
    {
        if(event->type() == QEvent::MouseButtonRelease)
        {
            QRect upRect;
            upRect.setTop(option.rect.top() + 3);
            upRect.setLeft(option.rect.right() - 15);
            upRect.setRight(option.rect.right() - 2);
            upRect.setBottom(option.rect.top() + 16);

            QRect downRect;
            downRect.setTop(option.rect.top() + 19);
            downRect.setLeft(option.rect.right() - 15);
            downRect.setRight(option.rect.right() - 2);
            downRect.setBottom(option.rect.top() + 32);

            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if(upRect.contains(mouseEvent->pos()))
            {
                emit upClicked(index.row());
            } else if (downRect.contains(mouseEvent->pos())) {
                emit downClicked(index.row());
            }
        }
    }
    else
    {
        if(event->type() == QEvent::MouseButtonRelease)
        {
            QRect minusRect;
            minusRect.setTop(option.rect.top() + 10);
            minusRect.setLeft(option.rect.left() + 240);
            minusRect.setRight(option.rect.left() + 255);
            minusRect.setBottom(option.rect.bottom() - 10);

            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if(minusRect.contains(mouseEvent->pos()))
            {
                emit minusClicked(index.row());
            }

        }
    }
    return false;
}

