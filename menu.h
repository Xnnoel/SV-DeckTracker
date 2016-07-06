#ifndef MENU_H
#define MENU_H

#include <QtGui>
#include <QMenu>
#include <QToolTip>

class Menu : public QMenu
{
    Q_OBJECT
public:
    Menu(){}
    bool event (QEvent * e)
    {
        const QHelpEvent *helpEvent = static_cast <QHelpEvent *>(e);
         if (helpEvent->type() == QEvent::ToolTip && activeAction() != 0)
         {
              QToolTip::showText(helpEvent->globalPos(), activeAction()->toolTip());
         } else
         {
              QToolTip::hideText();
         }
         return QMenu::event(e);
    }
};

#endif // MENU_H
