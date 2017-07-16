#include "frmwindow.h"
#include "carddelegate.h"
#include "card.h"
#include "QRgba64"
#include <QMouseEvent>
#include <QDir>


CardDelegate::CardDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    font = QFont("Segoe UI",10);
    font.setWeight(QFont::DemiBold);

    cardHeight = frmWindow::settingsMap["cardHeight"].toInt();
    cardWidth = frmWindow::settingsMap["cardWidth"].toInt();
    setColor(frmWindow::settingsMap["cardColor"]);

}

void CardDelegate::setPointers(svDatabase *db, cardlist * cd)

{
    database = db;
    playingDeck = cd;
    memset(cardEffect,0,sizeof(cardEffect));
}

void CardDelegate::setColor(QString str)
{
    if (!str.toInt())
        myColor = QColor(QRgba64::fromRgba(str.left(2).toInt(nullptr, 16),str.mid(2,2).toInt(nullptr, 16),str.right(2).toInt(nullptr, 16),0xff));
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

    QRect headerRect = option.rect;
    QRect subheaderRect = option.rect;
    QRect iconRect = option.rect;


    iconRect.setLeft(iconRect.left()- 3);
    iconRect.setRight(iconRect.left() + 32);
    headerRect.setLeft(iconRect.right());
    subheaderRect.setLeft(subheaderRect.right() - 20);
    headerRect.setRight(subheaderRect.left()-10);

    QStringList stringData = qvariant_cast<QStringList>(index.data());
    int myid = stringData[0].toInt();
    int mycount = stringData[1].toInt();

    Card card = database->getCard(myid);
    int mycost = card.manaCost;
    QString myname = card.name;

    /////// PAINTER HERE DRAW FUNCS ONLY
    // Get font width and shrink text
    QFontMetrics qm(font);

    int width = qm.width(myname);
    while (width > (headerRect.right() - headerRect.left()))
    {
        myname.remove(myname.size()-1,1);
        width = qm.width(myname);
    }

    //Draw background image of database
    QPixmap image = *(database->getPortrait(myid));
    painter->drawPixmap(QPoint(iconRect.left(), iconRect.top()) , image );
    QPixmap costimage = *(database->getCost(mycost));
    costimage = costimage.scaled(20,20);
    painter->drawPixmap(QPoint(iconRect.left() + iconRect.width()/2 - costimage.width()/2, iconRect.top() + iconRect.height()/2-costimage.height()/2) , costimage);

    //set color
    QColor color = QColor(255,255,255);
    if (mycount == 0)
        color = QColor(80,80,80);
    if (std::find(cardsInHand.begin(),cardsInHand.end(),myid)!= cardsInHand.end())
        color = myColor;

    //draw text
    painter->setPen(color);
    painter->setFont(font);
    painter->drawText(headerRect.left(), headerRect.top() + headerRect.height()/2 + font.pointSize()/2,myname);
    if (mycount > 0)
        painter->drawText(subheaderRect.left(), subheaderRect.top() + subheaderRect.height()/2 + font.pointSize()/2, QString::number(mycount));


    // draw if card effect active for row
    if (cardEffect[index.row()] > 0)
    {
        int alpha = 220 - qAbs(25 - cardEffect[index.row()]) * 5;
        QPainterPath path;
        path.addRect(option.rect);
        QColor temp(myColor);
        temp.setAlpha(alpha);
        painter->fillPath(path, temp);
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
    return(QSize(cardWidth, cardHeight));
}

bool CardDelegate::editorEvent(QEvent *event, QAbstractItemModel*, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    // Emit a signal when the icon is clicked
    if (!editMode)
    {
        if(deckFocused && event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

            if(option.rect.contains(mouseEvent->pos()))
            {
                if (mouseEvent->button() == Qt::LeftButton)
                    emit downClicked(index.row());
                else if (mouseEvent->button() == Qt::RightButton)
                    emit upClicked(index.row());
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

