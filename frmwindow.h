#ifndef FRMWINDOW_H
#define FRMWINDOW_H

#include <QMainWindow>
#include "perceptualhash.h"
#include "cardlist.h"
#include "svdatabase.h"
#include "svlistmodel.h"
#include "menu.h"

#include <QDir>
#include <vector>

#include <QListView>
#include <QStandardItemModel>
#include "carddelegate.h"

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

namespace Ui {
class frmWindow;
enum class STATE{MYTURN, FINDCARD};
}

class frmWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void update();

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

    void slotLoad();

    void slotSave();

    void slotHelp();
    void slotAbout();
    void slotContact();


private:
    Ui::frmWindow *ui;

    cv::Mat matTexture;
    ulong64 matTexturePhash;
    ulong64 theirTexturePhash;

    cv::Mat mat;

    HDC hdcScreen;
    HDC hdc;
    HBITMAP hbmp;
    HWND handle;
    bool cardFound;
    bool passed;

    SVListModel* model;

    // bounding box for window
    int height;
    int width;
    int left;
    int top;

    // bounding box for "MY TURN"
    int boxHeight;
    int boxWidth;
    int boxLeft;
    int boxTop;

    // bounding box for "MY TURN"
    int theirHeight;
    int theirWidth;
    int theirLeft;
    int theirTop;

    // bounding box for "cost"
    int costHeight;
    int costWidth;
    int costLeft;
    int costTop;

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
    QAction* LoadAction;
    QAction* SaveAction;
    //Help
    QAction* HelpAction;
    QAction* About;
    QAction* Contact;

    QDir dir;
    bool handleValid;

    Ui::STATE curState;
    void loadDeck(SVListModel* model);
    void sortDeck();

    // This is to hold all the info about the deck in the game
    cardlist playingDeck;

    // Database of all the cards in the game
    svDatabase cardDatabase;



    //debug stuff
    cv::Mat resultMat;
    int refreshRate;
    int ignoreNext;

    int turnDraw;
};

#endif // FRMWINDOW_H
