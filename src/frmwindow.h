#ifndef FRMWINDOW_H
#define FRMWINDOW_H

#include "cardlist.h"
#include "svdatabase.h"
#include "svlistmodel.h"
#include "menu.h"
#include "carddelegate.h"
#include "sveditmodel.h"
#include "editdelegate.h"
#include "processreader.h"

#include <vector>

#include <QMainWindow>
#include <QDir>
#include <QListView>
#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QPushButton>
#include <QCheckBox>

#include <QNetworkReply>

namespace Ui {
class frmWindow;
enum class STATE{MYTURN, FINDCARD};
}

class frmWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void signalGenerateDeck();

public slots:
    void update();
    void updateCount(int);
    void refreshList(int);
    void replyFinished(QNetworkReply*);
    void replyCodeFinished(QNetworkReply*);
    void clickedRow(int);

public:
    explicit frmWindow(QWidget *parent = 0);
    ~frmWindow();

private slots:
    void slotElf();
    void slotRoyal();
    void slotWitch();
    void slotDragon();
    void slotNecro();
    void slotVampire();
    void slotBishop();

    void slotClearDeck();
    void slotLoad();
    void slotLoadURL();

    void slotSaveAs();
    void slotSave();

    void slotHelp();
    void slotAbout();
    void slotSetBase();

    void slotButtonPushed();
    void slotEditMode();
    void slotLoadEdit(int);

private:
    Ui::frmWindow *ui;
    bool forceUpdateHash;

    bool needSave;
    double saveHash;

    void closeEvent(QCloseEvent *event);
    QMap<QString, QString> settingsMap;

    // Menu stuff
    void createActions();
    void createMenus();
    Menu *NewMenu;
    Menu *DeckMenu;
    Menu *HelpMenu;

    // Class Create New
    QAction* NewElf;
    QAction* NewRoyal;
    QAction* NewWitch;
    QAction* NewDragon;
    QAction* NewNecro;
    QAction* NewVampire;
    QAction* NewBishop;
    //Load/Save Decks
    QAction* ClearDeckAction;
    QAction* LoadAction;
    QAction* LoadURLAction;
    QAction* SaveAsAction;
    QAction* SaveAction;
    QAction* SetBaseAction;
    //Help
    QAction* HelpAction;
    QAction* About;

    QDir dir;
    bool handleValid;

    Ui::STATE curState;
    void loadDeck(SVListModel* model);
    void sortDeck();
    void setMyLayout();
    void createEditor();

    // This is to hold all the info about the deck in the game
    cardlist playingDeck;
    int blinker[40];

    // Database of all the cards in the game
    svDatabase cardDatabase;
    ProcessReader pr;
    bool last;
    std::vector<int> prevHand;

    // Stuff pertaining to grid layout
    QGridLayout* mainLayout;
    QLabel* labelDeckName;
    QLabel* labelBlank;
    QLabel* labelCards;

    QLineEdit* DeckNameEdit;
    QListView* PlayingDeckList;
    CardDelegate * delegate;
    SVListModel* model;
    QTimer* timer;

    QListView* EditDeckList;
    QPushButton* okButton;
    SVEditModel* editmodel;
    editDelegate* editdelegate;
    QCheckBox* neutralBox;
    QCheckBox* classBox;

    QPushButton* editButton;
    QTextEdit* turnLog;
};

#endif // FRMWINDOW_H
