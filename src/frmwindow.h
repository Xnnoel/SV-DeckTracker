#ifndef FRMWINDOW_H
#define FRMWINDOW_H

#include "perceptualhash.h"
#include "cardlist.h"
#include "svdatabase.h"
#include "svlistmodel.h"
#include "menu.h"
#include "carddelegate.h"
#include "sveditmodel.h"
#include "editdelegate.h"

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
    void updateCount(int);
    void refreshList(int);

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

    void slotSaveAs();
    void slotSave();

    void slotHelp();
    void slotAbout();

    void slotButtonPushed();
    void slotEditMode();
    void slotLoadEdit(int);

    void slotStart();
    void slotStop();
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
    bool passed;

    QMap<QString, QString> settingsMap;

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
    QAction* SaveAsAction;
    QAction* SaveAction;
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

    // Database of all the cards in the game
    svDatabase cardDatabase;

    // Stuff pertaining to grid layout
    QGridLayout* mainLayout;
    QLabel* label1;
    QLabel* label2;
    QLabel* label3;
    QLabel* label4;

    QLineEdit* DeckNameEdit;
    QPlainTextEdit* DeckDescEdit;
    QListView* PlayingDeckList;
    CardDelegate * delegate;
    SVListModel* model;
    QPushButton* startButton;
    QTimer* timer;


    QListView* EditDeckList;
    QPushButton* okButton;
    SVEditModel* editmodel;
    editDelegate* editdelegate;
    QCheckBox* neutralBox;
    QCheckBox* classBox;

    QPushButton* editButton;
    QPushButton* stopButton;
    QTextEdit* turnLog;

    int selectLeft[3];
    int selectTop;
    int selectWidth;
    int selectHeight;

    int resultsLeft;
    int resultsTop;
    int resultsWidth;
    int resultsHeight;

    ulong64 resultWinPhash;
    ulong64 resultLosePhash;

    //debug stuff
    cv::Mat resultMat;
    int refreshRate;
    int ignoreNext;

    int turnDraw;
};

#endif // FRMWINDOW_H
