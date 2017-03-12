#include "carddelegate.h"
#include "card.h"
#include <algorithm>
#include <QMouseEvent>
#include <QDir>


CardDelegate::CardDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void CardDelegate::setPointers(svDatabase *db, cardlist * cd)

{
    database = db;
    playingDeck = cd;
}

void CardDelegate::setCardsInHand(std::vector<int> cards)
{
    cardsInHand = cards;
}

void CardDelegate::blinkEffect(int row, int amount)
{
    cardEffect[row] = amount;
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

    iconRect.setRight(iconRect.left() + 40);
    headerRect.setLeft(iconRect.right());
    subheaderRect.setLeft(subheaderRect.right() - 20);

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
    if (mycount == 0)
        color = QColor(80,80,80);
    if (std::find(cardsInHand.begin(),cardsInHand.end(),myid)!= cardsInHand.end())
        color = QColor(200,200,0);

    painter->setPen(color);
    painter->setFont(font);
    painter->drawText(headerRect.left(), headerRect.top() + headerRect.height()/2 + font.pointSize()/2,myname);
    painter->drawText(subheaderRect.left(), subheaderRect.top() + subheaderRect.height()/2 + font.pointSize()/2, QString::number(mycount));

    // draw if card effect active for row
    if (cardEffect[index.row()] > 0)
    {
        int alpha = 220 - qAbs(20 - cardEffect[index.row()]) * 5;
        QPainterPath path;
        path.addRect(option.rect);
        color = QColor(200,200,0,alpha);
        painter->fillPath(path, color);
        painter->drawPath(path);
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
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if(option.rect.contains(mouseEvent->pos()))
            {
                emit downClicked(index.row());
            }
        }
    }
    else
    {
        if(event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if(option.rect.contains(mouseEvent->pos()))
            {
                emit minusClicked(index.row());
            }

        }
    }
    return false;
}

