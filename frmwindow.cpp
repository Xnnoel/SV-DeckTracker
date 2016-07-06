#include "frmwindow.h"
#include "ui_frmwindow.h"
#include "asmopencv.h"
#include "perceptualhash.h"
#include "cardlist.h"

#include <QtWinExtras/QtWin>
#include <QPixmap>
#include <QDirIterator>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>

#include <qtimer.h>
#include <Windows.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

Q_GUI_EXPORT QPixmap qt_pixmapFromWinHBITMAP(HBITMAP bitmap, int hbitmapFormat=0);

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Gdi32.lib")

std::wstring s2ws(const std::string& s);

frmWindow::frmWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmWindow),
    dir("."),
    playingDeck(&cardDatabase)
{
    ui->setupUi(this);

    // Set up some inits
    handle = 0;
    mat = 0;
    matTexture = 0;
    cardFound = false;

    //add menubar?
    createActions();
    createMenus();

    ///DEBUG
    /// ADD IN SOME SAMPLE CARDS FOR CURRENT DECK
    ///IDEALLY WE LOAD IN PLAYINGDECK, THEN COPY INTO MODEL
    playingDeck.setName("Midrange Royal?");
    playingDeck.setDesc("This is a description of the deck. Hopefully you can write tips and ideas here on what to evolve or something. I dunno.");
    //playingDeck.addCard(100211010);
    playingDeck.addCard(100211010);
    playingDeck.addCard(100211010);
    playingDeck.addCard(101232020);
    playingDeck.addCard(101211020);
    playingDeck.addCard(101211020);
    playingDeck.addCard(101211020);
    playingDeck.addCard(101211110);
    playingDeck.addCard(101211110);
    playingDeck.addCard(101221010);
    playingDeck.addCard(101221010);
    playingDeck.addCard(101221010);
    playingDeck.addCard(100214010);
    playingDeck.addCard(100211030);
    playingDeck.addCard(100211030);
    playingDeck.addCard(100211030);
    playingDeck.addCard(101211060);
    playingDeck.addCard(101211060);
    playingDeck.addCard(101211090);
    playingDeck.addCard(101211090);
    playingDeck.addCard(101211090);
    playingDeck.addCard(101221070);
    playingDeck.addCard(100211040);
    playingDeck.addCard(100211040);
    playingDeck.addCard(100221010);
    playingDeck.addCard(100221010);
    playingDeck.addCard(100221010);
    playingDeck.addCard(101211070);
    playingDeck.addCard(101211070);
    playingDeck.addCard(101221100);
    playingDeck.addCard(101031020);
    playingDeck.addCard(101031020);
    playingDeck.addCard(101024030);
    playingDeck.addCard(101241020);
    playingDeck.addCard(101241020);
    playingDeck.addCard(100221020);
    playingDeck.addCard(100221020);
    playingDeck.addCard(101241030);
    playingDeck.addCard(101234020);
    playingDeck.addCard(101231040);


    // Set up model view
    QListView *list =  ui->listView;
    model = new SVListModel;
    model->setPointer(&cardDatabase, &playingDeck);
    CardDelegate * delegate = new CardDelegate;
    delegate->setPointers(&cardDatabase, &playingDeck);

    connect(delegate, SIGNAL(upClicked(int)), model, SLOT(slotUp(int)));
    connect(delegate, SIGNAL(downClicked(int)), model, SLOT(slotDown(int)));
    connect(model, SIGNAL(countChanged(int)), this, SLOT(updateCount(int)));

    list->setModel(model);
    list->setItemDelegate(delegate);
    loadDeck(model);        //load data into model
    list->show();

    // Load in application settings
    QFile file(dir.absolutePath() + "/settings.ini");
    if (!file.open(QIODevice::ReadOnly))
        qWarning("Couldn't find settings.ini");

    QTextStream in(&file);

    QMap<QString, QString> settingsMap;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList splitLines = line.split("=");
        settingsMap.insert(splitLines[0],splitLines[1]);
    }
    file.close();

    std::string appName = settingsMap.value("Windowname").toStdString();
    std::wstring stemp = s2ws(appName);
    LPCWSTR result = stemp.c_str();

    handle = ::FindWindow(NULL, result);

    //load values we pulled from the file
    int topborder = settingsMap.value("Topborder").toInt();
    int botborder = settingsMap.value("Botborder").toInt();
    int leftborder = settingsMap.value("Leftborder").toInt();
    int rightborder = settingsMap.value("Rightborder").toInt();

    //Window rect
    RECT rc;
    GetClientRect(handle, &rc);
    width = (rc.right - rc.left) - rightborder - leftborder;
    height = (rc.bottom - rc.top) - topborder - botborder;
    top = rc.top+topborder;
    left = rc.left+leftborder;

    boxLeft = (int)round(0.261 * width) + left;
    boxTop = (int)round(0.341 * height) + top;
    boxWidth = (int)round(0.4735 * width);
    boxHeight = (int)round(0.1965 * height);

    theirLeft = (int)round(0.1636 * width) + left;
    theirTop = (int)round(0.3285 * height) + top;
    theirWidth = (int)round(0.6646 * width);
    theirHeight = (int)round(0.2206 * height);

    costLeft = (int)round(0.5795 * width) + left;
    costTop = (int)round(0.4461 * height) + top;
    costWidth = (int)round(0.02789 * width);
    costHeight = (int)round(0.04956 * height);

    // If we found the application, load this
    if (handle != 0)
    {
        // Start the update loop to check for cards in images
        refreshRate = 100;
        handleValid = true;

        //create bitmap and screen to save rect
        hdcScreen = GetDC(NULL);
        hdc = CreateCompatibleDC(hdcScreen);
        hbmp = CreateCompatibleBitmap(hdcScreen,
            width + leftborder, height + topborder);
        SelectObject(hdc, hbmp);

        //Guess the current state of the image
        curState = Ui::STATE::MYTURN;

        //Create a "you start" mat
        matTexture = cv::imread("pic.png");
        matTexturePhash = PerceptualHash::phash(matTexture);
        matTexture = cv::imread("pic2.png");
        theirTexturePhash = PerceptualHash::phash(matTexture);

        ignoreNext = 0;
        passed = false;
        turnDraw =0;

        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(update()));
        timer->start(refreshRate);

    }
}

frmWindow::~frmWindow()
{
    delete ui;
}

void frmWindow::sortDeck()
{
    for (int i = 1; i < playingDeck.cardsInDeck.size(); i++)
    {
        int j = i;

        while (j > 0 &&
               cardDatabase.getCard(playingDeck.cardsInDeck[j-1]).manaCost >
               cardDatabase.getCard(playingDeck.cardsInDeck[j]).manaCost)
        {
            int tempCard = playingDeck.cardsInDeck[j];
            int tempCount = playingDeck.countInDeck[j];
            ulong64 tempHash = playingDeck.deckPHash[j];
            playingDeck.cardsInDeck[j] = playingDeck.cardsInDeck[j-1];
            playingDeck.countInDeck[j] = playingDeck.countInDeck[j-1];
            playingDeck.deckPHash[j] = playingDeck.deckPHash[j-1];
            playingDeck.cardsInDeck[j-1] = tempCard;
            playingDeck.countInDeck[j-1] = tempCount;
            playingDeck.deckPHash[j-1] = tempHash;
            j--;
        }
    }
}

void frmWindow::loadDeck(SVListModel* model)
{
    //clear model first
    model->clearData();

    //sort the current deck before loading it into model
    sortDeck();

    //Push all cards from current deck into model
    for (int i = 0; i < playingDeck.cardsInDeck.size(); i++)
    {
        int id = playingDeck.cardsInDeck[i];
        int count = playingDeck.countInDeck[i];

        model->addCard(id);
        model->setCount(id, count);
    }
    int listsize = playingDeck.cardsInDeck.size() * 35 + 4;
    ui->listView->setFixedHeight(std::max(listsize,400));
    this->setFixedHeight(ui->listView->height() + 50);

    //Set text description
    ui->textDeckName->document()->setPlainText(QString::fromStdString(playingDeck.getName()));
    ui->textDeckDesc->document()->setPlainText(QString::fromStdString(playingDeck.getDescription()));

    int decksize = 0;
    for (int i = 0; i < playingDeck.countInDeck.size(); i++)
    {
        decksize += playingDeck.countInDeck[i];
    }

    updateCount(decksize);

}

void frmWindow::updateCount(int cardsize)
{
    int decksize = 0;
    for (int i = 0; i < playingDeck.countInDeck.size(); i++)
    {
        decksize += playingDeck.countInDeck[i];
    }

    QString cardCountLabel = QString::number(cardsize) + "/" + QString::number(decksize) + " Cards";
    ui->DeckCount->setText(cardCountLabel);
}

void frmWindow::update()
{
    /// In the update function, we will continous loop to try
    /// and guess what state the program is in. Mainly want to
    /// see if a game had started or not.
    //decrement all if exist
    if (ignoreNext > 0)ignoreNext--;


    //otherwise check for card
    if (handleValid && ::IsWindow(handle))
    {
        PrintWindow(handle, hdc, PW_CLIENTONLY);

        QPixmap pixmap = qt_pixmapFromWinHBITMAP(hbmp);

        QRect boxRect;
        QPixmap drawer;
        ulong64 imagePHash;

        if (ignoreNext > 0)
            ignoreNext--;

        int distance;

        switch (curState)
        {
        case Ui::STATE::MYTURN:
            //Print to memory hdc

            boxRect.setRect(boxLeft,boxTop,boxWidth,boxHeight);
            drawer = pixmap.copy(boxRect);
            mat = ASM::QPixmapToCvMat(drawer);
            imagePHash = PerceptualHash::phash(mat);

            distance = PerceptualHash::hammingDistance(matTexturePhash, imagePHash);


            if (distance < 20 && ignoreNext < 1)
            {
                setWindowTitle("Now it's my turn!");
                curState = Ui::STATE::FINDCARD;
                cardFound = false;
            }

            break;
        case Ui::STATE::FINDCARD:

            //Get the card cost (hopefully)
            //Try and find what the cost is?
            boxRect.setRect(costLeft,costTop,costWidth,costHeight);
            drawer = pixmap.copy(boxRect);
            mat = ASM::QPixmapToCvMat(drawer);
            imagePHash = PerceptualHash::phash(mat);

            std::vector<ulong64> numberPHash = cardDatabase.getCostPHash();

            PerceptualHash::ComparisonResult result = PerceptualHash::best(imagePHash, numberPHash);

            int cost = cardDatabase.getCostfromPHash(numberPHash[result.index]);
            int costDistance = result.distance;

            //Perspective shift on card for better readability (why are they tilted)
            mat = ASM::QPixmapToCvMat(pixmap);
            cv::Point2f input[4];
            cv::Point2f output[4];

            //in case of window size changed, use percentages // ADD SHIFT
            input[0] = cv::Point2f(0.6047f * width + left,0.8208f * height + top);
            input[1] = cv::Point2f(0.5968f * width + left,0.4958f * height + top);
            input[2] = cv::Point2f(0.7359f * width + left,0.4958f * height + top);
            input[3] = cv::Point2f(0.7492f * width + left,0.8194f * height + top);

            //doesnt matter here, so long as output size fits here
            output[0] = cv::Point2f(0,200);
            output[1] = cv::Point2f(0,0);
            output[2] = cv::Point2f(133,0);
            output[3] = cv::Point2f(133,200);

            cv::Mat lambda( 2, 4, CV_32FC1 );
            lambda = cv::Mat::zeros( mat.rows, mat.cols, mat.type() );

            lambda = cv::getPerspectiveTransform(input,output);
            cv::warpPerspective(mat, resultMat, lambda, cv::Size(133,200));

            imagePHash = PerceptualHash::phash(resultMat);

            std::vector<PerceptualHash::ComparisonResult> bestguess = PerceptualHash::nbest(3, imagePHash, playingDeck.deckPHash);

            if (ignoreNext > 0)
                setWindowTitle("Ignoring");
            else
                setWindowTitle("Looking for card");

            if (ignoreNext < 1 && (bestguess[0].distance < 15 || costDistance < 15))
            {
                //Matches best 3 to cost, hopefully one matches
                for (int i = 0; i < 3; i++)
                if (cardDatabase.getCard(playingDeck.cardsInDeck[bestguess[i].index]).manaCost == cost)
                {
                    model->subCard(playingDeck.cardsInDeck[bestguess[i].index]);
                    ignoreNext = (int)round(1000/refreshRate);
                    break;
                }
            }

            break;
        }
    }
    else
    {
        handleValid = false;
        setWindowTitle("Window is closed");
    }
}

std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

void frmWindow::createActions()
{
    NewElf = new QAction(tr("&Elf"), this);
    NewElf->setToolTip(tr("Create an elf deck"));
    connect(NewElf, &QAction::triggered, this, &frmWindow::slotElf);

    NewRoyal = new QAction(tr("&Royal"), this);
    NewRoyal->setToolTip(tr("Create an royal deck"));
    connect(NewRoyal, &QAction::triggered, this, &frmWindow::slotRoyal);

    NewWitch = new QAction(tr("&Witch"), this);
    NewWitch->setToolTip(tr("Create an Witch deck"));
    connect(NewWitch, &QAction::triggered, this, &frmWindow::slotWitch);

    NewDragon = new QAction(tr("&Dragon"), this);
    NewDragon->setToolTip(tr("Create an Dragon deck"));
    connect(NewDragon, &QAction::triggered, this, &frmWindow::slotDragon);

    NewNecro = new QAction(tr("&Necro"), this);
    NewNecro->setToolTip(tr("Create an Necro deck"));
    connect(NewNecro, &QAction::triggered, this, &frmWindow::slotNecro);

    NewVampire = new QAction(tr("&Vampire"), this);
    NewVampire->setToolTip(tr("Create an Vampire deck"));
    connect(NewVampire, &QAction::triggered, this, &frmWindow::slotVampire);

    NewBishop = new QAction(tr("&Bishop"), this);
    NewBishop->setToolTip(tr("Create an Bishop deck"));
    connect(NewBishop, &QAction::triggered, this, &frmWindow::slotBishop);

    LoadAction = new QAction(tr("&Load"), this);
    LoadAction->setToolTip(tr("Load a deck"));
    connect(LoadAction, &QAction::triggered, this, &frmWindow::slotLoad);

    SaveAction = new QAction(tr("&Save as..."), this);
    SaveAction->setToolTip(tr("Save current deck"));
    connect(SaveAction, &QAction::triggered, this, &frmWindow::slotSave);

    HelpAction = new QAction(tr("&Help"), this);
    HelpAction->setToolTip(tr("Open the help text"));
    connect(HelpAction, &QAction::triggered, this, &frmWindow::slotHelp);

    About = new QAction(tr("&About"), this);
    connect(About, &QAction::triggered, this, &frmWindow::slotAbout);

    Contact = new QAction(tr("&Contact"), this);
    connect(Contact, &QAction::triggered, this, &frmWindow::slotContact);

}


void frmWindow::createMenus()
{
    NewMenu = new Menu();
    NewMenu->setTitle(tr("&New"));
    menuBar()->addMenu(NewMenu);
    NewMenu->addAction(NewElf);
    NewMenu->addAction(NewRoyal);
    NewMenu->addAction(NewWitch);
    NewMenu->addAction(NewDragon);
    NewMenu->addAction(NewNecro);
    NewMenu->addAction(NewVampire);
    NewMenu->addAction(NewBishop);
    NewMenu->addSeparator();


    DeckMenu = new Menu();
    DeckMenu->setTitle(tr("&Load"));
    menuBar()->addMenu(DeckMenu);
    DeckMenu->addAction(LoadAction);
    DeckMenu->addAction(SaveAction);

    HelpMenu = new Menu();
    HelpMenu->setTitle(tr("&Help"));
    menuBar()->addMenu(HelpMenu);
    HelpMenu->addAction(HelpAction);
    HelpMenu->addSeparator();
    HelpMenu->addAction(About);
    HelpMenu->addAction(Contact);
}

void frmWindow::slotElf()
{
    //do nothing for now
}

void frmWindow::slotRoyal()
{
    //do nothing for now
}
void frmWindow::slotWitch()
{
    //do nothing for now
}
void frmWindow::slotDragon()
{
    //do nothing for now
}
void frmWindow::slotNecro()
{
    //do nothing for now
}
void frmWindow::slotVampire()
{
    //do nothing for now
}
void frmWindow::slotBishop()
{
    //do nothing for now
}
void frmWindow::slotLoad()
{

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Save Deck as"), dir.absolutePath() + "/Decks/NewDeck", tr("Image Files (*.dck)"));
    QFile loadfile(fileName);
    if (!loadfile.open(QIODevice::ReadOnly))
        qWarning("Couldn't open");

    QTextStream textStream(&loadfile);
    QString line = textStream.readLine();
    std::string deckname = line.toStdString();
    playingDeck.clear();
    line = textStream.readLine();
    std::string deckdesc;
    while (line != "--deck--")
    {
        deckdesc += line.toStdString();
        deckdesc += '\n';
        line = textStream.readLine();
    }
    line = textStream.readLine();

    while (line != "")
    {
        int ID = line.toInt();
        playingDeck.addCard(ID);
        line = textStream.readLine();
    }

    playingDeck.setName(deckname);
    playingDeck.setDesc(deckdesc);
    loadfile.close();
    loadDeck(model);
}
void frmWindow::slotSave()
{
    int decksize = 0;
    for (int i = 0; i < playingDeck.countInDeck.size(); i++)
    {
        decksize += playingDeck.countInDeck[i];
    }
    if(decksize < 40)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Save"), tr("You have less than 40 cards. Do you still want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No) {
             return;
        }
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Deck as"), dir.absolutePath() + "/Decks/NewDeck", tr("Decks (*.dck)"));
    QFile savefile(fileName);
    if (!savefile.open(QIODevice::WriteOnly)) {
           qWarning("Couldn't open save");
    }

    QString textname = ui->textDeckName->document()->toPlainText();
    playingDeck.setName(textname.toStdString());
    QString textdesc = ui->textDeckDesc->document()->toPlainText();
    playingDeck.setDesc(textdesc.toStdString());

    std::string final = playingDeck.getName() + '\n';
    savefile.write(final.c_str());
    std::string desc = playingDeck.getDescription() + '\n';
    savefile.write(desc.c_str());
    savefile.write("--deck--\n");
    for (int i = 0; i < playingDeck.cardsInDeck.size(); i++ )
    {
        for (int j = 0; j < playingDeck.countInDeck[i]; j++)
        {
            std::string writeID = std::to_string( playingDeck.cardsInDeck[i]) + '\n';
            savefile.write(writeID.c_str());
        }
    }
    savefile.close();

}
void frmWindow::slotAbout()
{
    //do nothing for now
}
void frmWindow::slotContact()
{
    //do nothing for now
}
void frmWindow::slotHelp()
{
    //do nothing for now
}
