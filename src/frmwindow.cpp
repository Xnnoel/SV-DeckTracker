#include "frmwindow.h"
#include "ui_frmwindow.h"
#include "asmopencv.h"

#include <QtWinExtras/QtWin>
#include <QPixmap>
#include <QDirIterator>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>

#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>

#include <qtimer.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Gdi32.lib")

#define WINWIDTH 350
#define EDITWINWIDTH 500
#define PORTRAITHEIGHT 35

frmWindow::frmWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmWindow),
    dir("."),
    playingDeck(&cardDatabase)
{
    ui->setupUi(this);

    // Set up some inits
    mat = 0;
    matTexture = 0;
    setWindowTitle("Shadowverse Deck Tracker");
    needSave = false;
    saveHash = 0;
    handleValid = false;

    //add menubar?
    setMyLayout();
    createActions();
    createMenus();

    // Set up model view
    model = new SVListModel;
    model->setPointer(&cardDatabase, &playingDeck);
    delegate = new CardDelegate;
    delegate->setPointers(&cardDatabase, &playingDeck);
    delegate->editMode = false;

    connect(delegate, SIGNAL(upClicked(int)), model, SLOT(slotUp(int)));
    connect(delegate, SIGNAL(downClicked(int)), model, SLOT(slotDown(int)));
    connect(model, SIGNAL(countChanged(int)), this, SLOT(updateCount(int)));

    PlayingDeckList->setModel(model);
    PlayingDeckList->setItemDelegate(delegate);
    PlayingDeckList->show();

    loadDeck(model);        //load data into model

    //set up edit model view and stuff
    editmodel = new SVEditModel;
    editmodel->setPointer(&cardDatabase, &playingDeck, model);
    editdelegate = new editDelegate;
    editdelegate->setPointers(&cardDatabase, &playingDeck);

    connect(editdelegate, SIGNAL(plusClicked(int)), editmodel, SLOT(slotPlusRow(int)));
    connect(delegate, SIGNAL(minusClicked(int)), model, SLOT(slotMinusRow(int)));
    connect(model, SIGNAL(deckChanged(int)), this, SLOT(refreshList(int)));
    connect(editmodel, SIGNAL(deckChanged(int)), this, SLOT(refreshList(int)));

    EditDeckList->setModel(editmodel);
    EditDeckList->setItemDelegate(editdelegate);
    EditDeckList->setHidden(true);
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
    setFixedWidth(WINWIDTH);

    int listsize = playingDeck.cardsInDeck.size() * PORTRAITHEIGHT + 4;
    PlayingDeckList->setFixedHeight(std::max(listsize,0));
    PlayingDeckList->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored));
    setFixedHeight(400);
    //Set text description
    DeckNameEdit->setText(QString::fromStdString(playingDeck.getName()));
    int decksize = playingDeck.getDeckSize();

    updateCount(decksize);

}

void frmWindow::updateCount(int cardsize)
{
    int decksize = playingDeck.getDeckSize();

    QString cardCountLabel = QString::number(cardsize) + "/" + QString::number(decksize) + " Cards";
    labelCards->setText(cardCountLabel);
}

void frmWindow::update()
{
    /// In the update function, we will continous loop to try
    /// and guess what state the program is in. Mainly want to
    /// see if a game had started or not.
    //otherwise check for card
    if (handleValid && ::IsWindow(handle))
    {

        QRect boxRect;
        QPixmap drawer;

        switch (curState)
        {
            case Ui::STATE::MYTURN:
            {
            /*
                // Get the best match for 3 cards on screen
                for (int i = 0; i < 3; i++)
                {
                    boxRect.setRect(selectLeft[i],selectTop,selectWidth,selectHeight);
                    drawer = pixmap.copy(boxRect);
                    resultMat = ASM::QPixmapToCvMat(drawer);
                    imagePHash = PerceptualHash::phash(resultMat);
                    PerceptualHash::ComparisonResult result = PerceptualHash::best(imagePHash, playingDeck.deckPHash);
                    if (result.distance < 18)
                    {
                        bestID[i+3] = bestID[i];
                        bestID[i] = playingDeck.cardsInDeck[result.index];
                    }
                }

                // My turn scanbox
                boxRect.setRect(boxLeft,boxTop,boxWidth,boxHeight);
                drawer = pixmap.copy(boxRect);
                mat = ASM::QPixmapToCvMat(drawer);
                imagePHash = PerceptualHash::phash(mat);

                int distance1 = PerceptualHash::hammingDistance(matTexturePhash, imagePHash);

                // Their turn scanbox
                boxRect.setRect(theirLeft,theirTop,theirWidth,theirHeight);
                drawer = pixmap.copy(boxRect);
                resultMat = ASM::QPixmapToCvMat(drawer);
                imagePHash = PerceptualHash::phash(resultMat);

                int distance2 = PerceptualHash::hammingDistance(theirTexturePhash, imagePHash);
                int distanceSens = settingsMap.value("TurnSens").toInt();

                setWindowTitle(QString::number(distance1) + ", " + QString::number(distance2));
                if (distance1 < distanceSens || distance2 < distanceSens)
                {
                    cv::imwrite("myturn.jpg", resultMat);
                    cv::imwrite("theirturn.jpg", mat);

                    //Remove guessed cards from active list
                    turnLog->append("Started with...");
                    for (int i = 0; i < 3; i++)
                        if (bestID[i] > 0)
                        {
                            model->subCard(bestID[i]);
                            QString cardname = cardDatabase.getCard(bestID[i]).name;
                            turnLog->append(cardname + ',');
                        }
                    turnLog->append("-------------");
                    curState = Ui::STATE::FINDCARD;
                }
*/
                break;
            }
            case Ui::STATE::FINDCARD:
            {
            /*
                if (ignoreNext > 0)
                    ignoreNext--;
                if (ignoreNext < 1){
                    //First check for matching win/lose screen
                    boxRect.setRect(resultsLeft,resultsTop,resultsWidth,resultsHeight);
                    drawer = pixmap.copy(boxRect);
                    mat = ASM::QPixmapToCvMat(drawer);
                    imagePHash = PerceptualHash::phash(mat);
                    int victoryDistance = PerceptualHash::hammingDistance(resultWinPhash, imagePHash);
                    int loseDistance = PerceptualHash::hammingDistance(resultLosePhash, imagePHash);

                    int gameEndDist = settingsMap.value("GameEndSens").toInt();
                    if (victoryDistance < gameEndDist || loseDistance < gameEndDist)
                    {
                        turnLog->append("Game ended\n----Resetting deck---");
                        loadDeck(model);
                        curState = Ui::STATE::MYTURN;
                        break;
                    }

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
                    std::vector<PerceptualHash::ComparisonResult> bestguess = PerceptualHash::nbest(NBEST, imagePHash, playingDeck.deckPHash);

                    // Check in case of draw second
                    input[0] = cv::Point2f(0.23767f * width + left,0.69372f * height + top);
                    input[1] = cv::Point2f(0.25754f * width + left,0.29712f * height + top);
                    input[2] = cv::Point2f(0.43855f * width + left,0.30105f * height + top);
                    input[3] = cv::Point2f(0.43341f * width + left,0.69372f * height + top);

                    lambda = cv::Mat::zeros( mat.rows, mat.cols, mat.type() );
                    lambda = cv::getPerspectiveTransform(input,output);
                    cv::warpPerspective(mat, resultMat, lambda, cv::Size(133,200));
                    imagePHash = PerceptualHash::phash(resultMat);
                    PerceptualHash::ComparisonResult bestguess2 = PerceptualHash::best(imagePHash, playingDeck.deckPHash);

                    // Once more for part 3
                    input[0] = cv::Point2f(0.5688f * width + left,0.69372f * height + top);
                    input[1] = cv::Point2f(0.56439f * width + left,0.29974f * height + top);
                    input[2] = cv::Point2f(0.74099f * width + left,0.30236f * height + top);
                    input[3] = cv::Point2f(0.76012f * width + left,0.69372f * height + top);

                    lambda = cv::Mat::zeros( mat.rows, mat.cols, mat.type() );
                    lambda = cv::getPerspectiveTransform(input,output);
                    cv::warpPerspective(mat, resultMat, lambda, cv::Size(133,200));
                    imagePHash = PerceptualHash::phash(resultMat);
                    PerceptualHash::ComparisonResult bestguess3 = PerceptualHash::best(imagePHash, playingDeck.deckPHash);



                    if ((bestguess[0].distance < settingsMap.value("BestGuessSens").toInt() || costDistance < settingsMap.value("CostGuessSens").toInt() )
                            && bestguess[0].distance < bestguess2.distance && bestguess[0].distance < bestguess3.distance)
                    {
                        // if card art is way off, cost could randomly spike below
                        if (bestguess[0].distance > settingsMap.value("WorstGuessSens").toInt())
                            break;

                        int cardConf = 6 * std::min(10, std::max(0, settingsMap.value("BestGuessSens").toInt() - (bestguess[0].distance-5)));
                        int costConf = 4 * std::min(10, std::max(0, settingsMap.value("CostGuessSens").toInt() - (costDistance-5)));
                        int confidence = cardConf + costConf;
                        // if card art matches well, assume that the card is true
                        if (bestguess[0].distance <= settingsMap.value("BestGuessSens").toInt())
                        {
                            model->subCard(playingDeck.cardsInDeck[bestguess[0].index]);
                            QString cardname = cardDatabase.getCard(playingDeck.cardsInDeck[bestguess[0].index]).name;
                            turnLog->append("Drew " + cardname + ", Confidence = "+ QString::number(confidence)+ "\n-------");
                            ignoreNext = (int)round(700/refreshRate);
                            break;
                        }

                        //Matches best 3 to cost, hopefully one matches
                        for (int i = 0; i < NBEST; i++)
                        if (cardDatabase.getCard(playingDeck.cardsInDeck[bestguess[i].index]).manaCost == cost)
                        {
                            model->subCard(playingDeck.cardsInDeck[bestguess[i].index]);
                            QString cardname = cardDatabase.getCard(playingDeck.cardsInDeck[bestguess[i].index]).name;
                            turnLog->append("Drew " + cardname + ", Confidence = "+ QString::number(confidence)+ "\n-------");
                            ignoreNext = (int)round(700/refreshRate);
                            break;
                        }
                    }
                    else if ((bestguess2.distance < settingsMap.value("BestGuessSens").toInt() && (bestguess3.distance - 5) < settingsMap.value("BestGuessSens").toInt()) ||
                             bestguess3.distance < settingsMap.value("BestGuessSens").toInt() && (bestguess2.distance - 5) < settingsMap.value("BestGuessSens").toInt())
                    {
                        model->subCard(playingDeck.cardsInDeck[bestguess2.index]);
                        model->subCard(playingDeck.cardsInDeck[bestguess3.index]);
                        QString cardname = cardDatabase.getCard(playingDeck.cardsInDeck[bestguess2.index]).name;
                        QString cardname2 = cardDatabase.getCard(playingDeck.cardsInDeck[bestguess3.index]).name;
                        turnLog->append("Drew " + cardname + " + " + cardname2 + ", num = " + QString::number(bestguess2.distance) + "," + QString::number(bestguess3.distance) + "\n-------");
                        cv::imwrite("draw3.jpg", resultMat);
                        cv::imwrite("everything.jpg", mat);
                        ignoreNext = (int)round(700/refreshRate);
                        break;
                    }

                }
                */
                break;
            }
        }
    }
    else
    {
        handleValid = false;
        setWindowTitle("Can't Find Window...");
        slotStop();
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

    LoadAction = new QAction(tr("&Load from file"), this);
    LoadAction->setToolTip(tr("Load a deck"));
    connect(LoadAction, &QAction::triggered, this, &frmWindow::slotLoad);

    LoadURLAction = new QAction(tr("&Load from url"), this);
    LoadURLAction->setToolTip(tr("Load using shadowportal URL"));
    connect(LoadURLAction, &QAction::triggered, this, &frmWindow::slotLoadURL);

    SaveAsAction = new QAction(tr("&Save deck as..."), this);
    SaveAsAction->setToolTip(tr("Save current deck as"));
    connect(SaveAsAction, &QAction::triggered, this, &frmWindow::slotSaveAs);

    SaveAction = new QAction(tr("&Save deck"), this);
    SaveAction->setToolTip(tr("Save current deck"));
    SaveAction->setShortcuts(QKeySequence::Save);
    connect(SaveAction, &QAction::triggered, this, &frmWindow::slotSave);

    HelpAction = new QAction(tr("&Help"), this);
    HelpAction->setToolTip(tr("Open the help text"));
    connect(HelpAction, &QAction::triggered, this, &frmWindow::slotHelp);

    HelpAction = new QAction(tr("&Help"), this);
    HelpAction->setToolTip(tr("Open the help text"));
    connect(HelpAction, &QAction::triggered, this, &frmWindow::slotHelp);


    About = new QAction(tr("&About"), this);
    connect(About, &QAction::triggered, this, &frmWindow::slotAbout);

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
    DeckMenu->setTitle(tr("&Deck"));
    menuBar()->addMenu(DeckMenu);
    DeckMenu->addAction(LoadAction);
    DeckMenu->addAction(LoadURLAction);
    DeckMenu->addSeparator();
    DeckMenu->addAction(SaveAction);
    DeckMenu->addAction(SaveAsAction);

    HelpMenu = new Menu();
    HelpMenu->setTitle(tr("&Help"));
    menuBar()->addMenu(HelpMenu);
    HelpMenu->addAction(HelpAction);
    HelpMenu->addSeparator();
    HelpMenu->addAction(About);
}

void frmWindow::slotElf()
{
    if (needSave)
    {
        //do some slot
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Unsaved Deck"), tr("The deck is unsaved. Do you want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            slotSave();
            return;
        }
    }
    playingDeck.clear();
    playingDeck.setClass(0);
    loadDeck(model);
    createEditor(); //spawns an editor on the right side
    slotLoadEdit(0);
}

void frmWindow::slotRoyal()
{
    if (needSave)
    {
        //do some slot
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Unsaved Deck"), tr("The deck is unsaved. Do you want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            slotSave();
            return;
        }
    }
    playingDeck.clear();
    playingDeck.setClass(1);
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
}

void frmWindow::slotWitch()
{
    if (needSave)
    {
        //do some slot
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Unsaved Deck"), tr("The deck is unsaved. Do you want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            slotSave();
            return;
        }
    }
    playingDeck.clear();
    playingDeck.setClass(2);
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
}

void frmWindow::slotDragon()
{
    if (needSave)
    {
        //do some slot
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Unsaved Deck"), tr("The deck is unsaved. Do you want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            slotSave();
            return;
        }
    }
    playingDeck.clear();
    playingDeck.setClass(3);
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
}

void frmWindow::slotNecro()
{
    if (needSave)
    {
        //do some slot
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Unsaved Deck"), tr("The deck is unsaved. Do you want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            slotSave();
            return;
        }
    }
    playingDeck.clear();
    playingDeck.setClass(4);
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
}

void frmWindow::slotVampire()
{
    if (needSave)
    {
        //do some slot
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Unsaved Deck"), tr("The deck is unsaved. Do you want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            slotSave();
            return;
        }
    }
    playingDeck.clear();
    playingDeck.setClass(5);
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
}

void frmWindow::slotBishop()
{
    if (needSave)
    {
        //do some slot
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Unsaved Deck"), tr("The deck is unsaved. Do you want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            slotSave();
            return;
        }
    }
    playingDeck.clear();
    playingDeck.setClass(6);
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
}

void frmWindow::slotLoad()
{
    if (needSave)
    {
        //do some slot
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Unsaved Deck"), tr("The deck is unsaved. Do you want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            slotSave();
            return;
        }
    }
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Deck"), dir.absolutePath() + "/Decks/NewDeck", tr("Image Files (*.dck)"));
    QFile loadfile(fileName);
    if (!loadfile.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open from slotload");
        return;
    }
    QTextStream textStream(&loadfile);
    QString line = textStream.readLine();
    std::string deckname = line.toStdString();
    playingDeck.clear();
    line = textStream.readLine();
    int classnum = std::stoi(line.toStdString());
    line = textStream.readLine();
    while (line != "")
    {
        int ID = line.toInt();
        playingDeck.addCard(ID);
        line = textStream.readLine();
    }

    playingDeck.setName(deckname);
    playingDeck.setClass(classnum);
    playingDeck.setFileName(fileName);
    DeckNameEdit->setText(QString::fromStdString(playingDeck.getName()));
    loadfile.close();
    loadDeck(model);
    delegate->editMode = false;
    mainLayout->removeWidget(EditDeckList);
    mainLayout->removeWidget(okButton);
    mainLayout->removeWidget(neutralBox);
    mainLayout->removeWidget(classBox);
    mainLayout->addWidget(editButton,0,2);
    editButton->setGeometry(0,0,0,0);
    EditDeckList->setGeometry(0,0,0,0);
    okButton->setGeometry(0,0,0,0);
    neutralBox->setGeometry(0,0,0,0);
    classBox->setGeometry(0,0,0,0);
    setFixedWidth(WINWIDTH);
    needSave = false;
}

void frmWindow::slotLoadURL()
{
    if (needSave)
    {
        //do some slot
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Unsaved Deck"), tr("The deck is unsaved. Do you want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            slotSave();
            return;
        }
    }

    bool ok;
    QString text = QInputDialog::getText(this, tr(""),
                                    tr("Shadowverse Portal link:"), QLineEdit::Normal,
                                    "", &ok,Qt::WindowSystemMenuHint | Qt::WindowTitleHint);

    if (ok && !text.isEmpty())
    {
        if (text.contains("shadowverse-portal.com/deckbuilder/create/"))
        {
            QStringRef urlHash(&text,57,text.length()-57);
            text = "https://shadowverse-portal.com/deck/" + urlHash.toString();
        }

        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(replyFinished(QNetworkReply*)));
        QUrl url(text);
        QNetworkRequest req(url);
        manager->get(req);

        //Create a dialog box while building deck (prevent user from touching)
        QMessageBox *msgBox = new QMessageBox(QMessageBox::NoIcon, "", "Building Deck...");
        msgBox->setStandardButtons(QMessageBox::NoButton);
        msgBox->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint);
        connect(this, SIGNAL(signalGenerateDeck()), msgBox, SLOT(reject()));
        msgBox->exec();
    }
    needSave = true;
}

void frmWindow::slotSaveAs()
{
    int decksize = playingDeck.getDeckSize();
    if(decksize < 40)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Save"), tr("There are less than 40 cards in the deck. Do you still want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No) {
             return;
        }
    }
    QString textname = DeckNameEdit->text();
    QString fileTextName = "NewDeck";
    if (!textname.isEmpty())
        fileTextName = textname.replace(" ","_");
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Deck as"), dir.absolutePath() + "/Decks/" + fileTextName, tr("Decks (*.dck)"));
    QFile savefile(fileName);
    if (!savefile.open(QIODevice::WriteOnly)) {
           qWarning("Couldn't open save from slotSaveAs");
           return;
    }
    playingDeck.setFileName(fileName);
    playingDeck.setName(textname.toStdString());

    std::string final = playingDeck.getName() + '\n';
    savefile.write(final.c_str());
    std::string classnum = std::to_string(playingDeck.getClass()) + '\n';
    savefile.write(classnum.c_str());
    for (int i = 0; i < playingDeck.cardsInDeck.size(); i++ )
    {
        for (int j = 0; j < playingDeck.countInDeck[i]; j++)
        {
            std::string writeID = std::to_string( playingDeck.cardsInDeck[i]) + '\n';
            savefile.write(writeID.c_str());
        }
    }
    savefile.close();
    needSave = false;
}

void frmWindow::slotSave()
{
    if (playingDeck.getFileName() == "" || playingDeck.getFileName().isEmpty())
    {
        slotSaveAs();
        return;
    }

    int decksize = playingDeck.getDeckSize();
    if(decksize < 40)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Save"), tr("There are less than 40 cards in the deck. Do you still want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No) {
             return;
        }
    }

    QFile savefile(playingDeck.getFileName());
    if (!savefile.open(QIODevice::WriteOnly)) {
           qWarning("Couldn't open save from slotSave");
           return;
    }

    QString textname = DeckNameEdit->text();
    playingDeck.setName(textname.toStdString());

    std::string final = playingDeck.getName() + '\n';
    savefile.write(final.c_str());
    std::string classnum = std::to_string(playingDeck.getClass()) + '\n';
    savefile.write(classnum.c_str());
    for (int i = 0; i < playingDeck.cardsInDeck.size(); i++ )
    {
        for (int j = 0; j < playingDeck.countInDeck[i]; j++)
        {
            std::string writeID = std::to_string( playingDeck.cardsInDeck[i]) + '\n';
            savefile.write(writeID.c_str());
        }
    }
    savefile.close();
    needSave = false;
}

void frmWindow::slotAbout()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::information(this, tr("About SV Deck Tracker"),
                                     tr("Shadowverse Deck Tracker\n"
                                        "Version 0.8.2\n"
                                        "\n"
                                        "For any comments or questions,\n"
                                        "Send an email to xnnoelx@gmail.com"));
}

void frmWindow::slotHelp()
{
    //do nothing for now
    QProcess *proc = new QProcess(this);
    QString path = dir.absolutePath() + "/data/help.txt";
    proc->start("notepad.exe "+path);
}


void frmWindow::setMyLayout()
{
    //layout of all those things yay
    mainLayout = new QGridLayout();
    labelDeckName = new QLabel("Deck Name");
    labelDeckName->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    labelDeckName->setMaximumHeight(25);

    labelBlank = new QLabel("");
    labelBlank->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

    labelCards = new QLabel("Cards");
    labelCards->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    labelCards->setMaximumHeight(25);
    labelCards->setAutoFillBackground(true);

    DeckNameEdit = new QLineEdit();
    DeckNameEdit->setMaximumHeight(25);
    DeckNameEdit->setMaxLength(35);
    DeckNameEdit->setPlaceholderText(tr(" Deck name here"));

    PlayingDeckList = new QListView();
    PlayingDeckList->setFixedWidth(230);
    PlayingDeckList->setWindowTitle("Deck List");
    PlayingDeckList->setWindowFlags(Qt::WindowTitleHint);

    startButton = new QPushButton("Start");
    editButton = new QPushButton("Edit");
    stopButton = new QPushButton("Stop");
    connect(editButton, SIGNAL(released()), this, SLOT(slotEditMode()));
    connect(startButton,SIGNAL(released()), this, SLOT(slotStart()));
    connect(stopButton, SIGNAL(released()), this, SLOT(slotStop()));

    mainLayout->addWidget(labelDeckName, 0, 0);
    mainLayout->addWidget(DeckNameEdit, 1, 0);
    mainLayout->addWidget(startButton,2,0);
    mainLayout->addWidget(labelBlank, 3, 0);
    mainLayout->addWidget(labelCards, 0, 1);
    mainLayout->addWidget(editButton, 0, 2);

    ui->centralWidget->setLayout(mainLayout);

    //set up extra layout here
    EditDeckList = new QListView();
    EditDeckList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    EditDeckList->setFixedWidth(280);
    okButton = new QPushButton("Done");
    connect(okButton, SIGNAL (released()),this, SLOT (slotButtonPushed()));

    neutralBox = new QCheckBox("Neutral");
    classBox = new QCheckBox("Class");
    connect(neutralBox, SIGNAL(stateChanged(int)),this ,SLOT(slotLoadEdit(int)));
    connect(classBox, SIGNAL(stateChanged(int)),this ,SLOT(slotLoadEdit(int)));

    turnLog = new QTextEdit();
    turnLog->setReadOnly(true);;
}

void frmWindow::createEditor()
{
    // This spawns an editor to the right side that you can use to click on the deck with
    mainLayout->addWidget(okButton, 0, 2);
    mainLayout->addWidget(neutralBox, 0, 3);
    mainLayout->addWidget(classBox, 0, 4);
    mainLayout->addWidget(EditDeckList, 1, 1,5,4);
    mainLayout->removeWidget(editButton);
    mainLayout->removeWidget(startButton);
    editButton->setGeometry(0,0,0,0);
    startButton->setGeometry(0,0,0,0);
    neutralBox->setChecked(true);
    classBox->setChecked(true);

    setFixedWidth(EDITWINWIDTH);

    delegate->editMode = true;
    PlayingDeckList->setFixedHeight(600);
    setFixedHeight(700);

    menuBar()->setEnabled(false);
}

void frmWindow::slotButtonPushed()
{
    mainLayout->removeWidget(EditDeckList);
    mainLayout->removeWidget(okButton);
    mainLayout->removeWidget(neutralBox);
    mainLayout->removeWidget(classBox);
    mainLayout->addWidget(editButton,0,2);
    mainLayout->addWidget(startButton,2,0);
    EditDeckList->setGeometry(0,0,0,0);
    okButton->setGeometry(0,0,0,0);
    neutralBox->setGeometry(0,0,0,0);
    classBox->setGeometry(0,0,0,0);
    setFixedWidth(WINWIDTH);
    delegate->editMode = false;
    loadDeck(model);
    menuBar()->setEnabled(true);

    if (saveHash != playingDeck.makeDeckHash())
        needSave = true;
}

void frmWindow::slotLoadEdit(int)
{
    editmodel->clearCards();

    int subClass = playingDeck.getClass();


    if (neutralBox->isChecked())
    {
        //Load nuetrals into edit
        for (int i = 0; i < 8; i ++)
            editmodel->addCard(cardDatabase.cardID[i]);

        for (int i = 85; i < 109; i++)
            editmodel->addCard(cardDatabase.cardID[i]);
    }

    if (classBox->isChecked())
    {
        switch(subClass)
        {
        case 0: //elf
            for (int i = 8; i < 19; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 109; i < 151; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        case 1: //royal
            for (int i = 19; i < 30; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 151; i < 193; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        case 2: //witch
            for (int i = 30; i < 41; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 193; i < 235; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        case 3: //dragon
            for (int i = 41; i < 52; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 235; i < 277; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        case 4: //necro
            for (int i = 52; i < 63; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 277; i < 319; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        case 5: //blood
            for (int i = 63; i < 74; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 319; i < 361; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        case 6: //bishop
            for (int i = 74; i < 85; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 361; i < 403; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        }
    }
    //sort editmodel?
    editmodel->sortList();
    EditDeckList->setHidden(false);
}

void frmWindow::refreshList(int meh)
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
    updateCount(meh);
}

void frmWindow::slotEditMode()
{
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
    saveHash = playingDeck.makeDeckHash();
}

void frmWindow::slotStart()
{

    //Begin the app loop
    handle = 0;
    std::string appName = "Shadowverse";
    std::wstring stemp = s2ws(appName);
    LPCWSTR result = stemp.c_str();
    handle = ::FindWindow(NULL, result);

    //load values we pulled from the file
    int topborder = settingsMap.value("Topborder").toInt();
    int botborder = settingsMap.value("Botborder").toInt();
    int leftborder = settingsMap.value("Leftborder").toInt();
    int rightborder = settingsMap.value("Rightborder").toInt();

    //Add some values for other settings for now, maybe add slider later?
    settingsMap.insert("TurnSens","22");
    settingsMap.insert("GameEndSens","15");
    settingsMap.insert("BestGuessSens","18");
    settingsMap.insert("WorstGuessSens","25");
    settingsMap.insert("CostGuessSens","18");

    //Window rect
    RECT rc;
    GetClientRect(handle, &rc);
    width = (rc.right - rc.left) - rightborder - leftborder;
    height = (rc.bottom - rc.top) - topborder - botborder;
    top = rc.top+topborder;
    left = rc.left+leftborder;

    boxLeft = (int)round(0.2617 * width) + left;
    boxTop = (int)round(0.3528 * height) + top;
    boxWidth = (int)round(0.468 * width);
    boxHeight = (int)round(0.1777 * height);

    theirLeft = (int)round(0.1742 * width) + left;
    theirTop = (int)round(0.3514 * height) + top;
    theirWidth = (int)round(0.6508 * width);
    theirHeight = (int)round(0.1736 * height);

    costLeft = (int)round(0.5792 * width) + left;
    costTop = (int)round(0.4465 * height) + top;
    costWidth = (int)round(0.02825 * width);
    costHeight = (int)round(0.05020 * height);

    resultsLeft = (int)round(0.3773 * width) + left;
    resultsTop = (int)round(0.06805 * height) + top;
    resultsWidth = (int)round(0.2453 * width);
    resultsHeight = (int)round(0.08472 * height);

    selectLeft[0] = (int) round(0.22169 * width) + left;
    selectLeft[1] = (int) round(0.44337 * width) + left;
    selectLeft[2] = (int) round(0.66484 * width) + left;
    selectTop = (int) round(0.64591 * height) + top;
    selectWidth = (int) round(0.1133 * width);
    selectHeight = (int) round(0.2515 * height);

    // If we found the application, load this
    if (handle != 0)
    {
        setWindowTitle("Shadowverse Deck Tracker");

        PlayingDeckList->raise();
        handleValid = true;

        //create bitmap and screen to save rect
        hdcScreen = GetDC(NULL);
        hdc = CreateCompatibleDC(hdcScreen);
        hbmp = CreateCompatibleBitmap(hdcScreen,
            width + leftborder, height + topborder);
        SelectObject(hdc, hbmp);

        //Guess the current state of the image
        curState = Ui::STATE::MYTURN;

        passed = false;
        turnDraw =0;

        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(update()));

        //Remove edit button
        mainLayout->removeWidget(editButton);
        editButton->setGeometry(0,0,0,0);

        //remove start button
        mainLayout->removeWidget(startButton);
        startButton->setGeometry(0,0,0,0);

        //add stop button
        mainLayout->addWidget(stopButton,4,0);

        //replace blank space with turn log
        mainLayout->addWidget(turnLog,5,0);
        mainLayout->removeWidget(labelBlank);
        labelBlank->setGeometry(0,0,0,0);
        turnLog->clear();
        turnLog->append("Draw Log\n******************\n");
        menuBar()->setEnabled(false);

        loadDeck(model);
    }
    else
    {
        QMessageBox::StandardButton errorMessage;
        errorMessage = QMessageBox::information(this, tr(""),
                                         tr("Could not find the application window."));
    }
}

void frmWindow::slotStop()
{
    timer->stop();
    mainLayout->removeWidget(stopButton);
    stopButton->setGeometry(0,0,0,0);
    mainLayout->addWidget(startButton,2,0);
    mainLayout->addWidget(editButton,0,2);

    mainLayout->removeWidget(turnLog);
    turnLog->setGeometry(0,0,0,0);
    mainLayout->addWidget(labelBlank,5,0);

    menuBar()->setEnabled(true);
    loadDeck(model);
}

void frmWindow::replyFinished(QNetworkReply * reply)
{
    if ( reply->error() != QNetworkReply::NoError ) {
        QMessageBox::StandardButton errorMessage;
        errorMessage = QMessageBox::information(this, tr(""),
                                         tr("An error has occured. Please provide a proper URL."));
        emit signalGenerateDeck();
    }
    else
    {
        std::vector<QString> deckVector;

        // Extract number and card from http page
        int classType;
        char buf[1024];

        while (!reply->atEnd())
        {
            qint64 lineLength = reply->readLine(buf, sizeof(buf));
            if (lineLength != -1)
            {
                QString newLine = QString(buf);

                if (newLine.contains("el-card-list-info-count"))
                {
                    QStringRef numberLine(&newLine,36,1);
                    deckVector.push_back(numberLine.toString());
                }

                if (newLine.contains("\"/card/"))
                {
                    QStringRef numberLine(&newLine,12,9);
                    deckVector.push_back(numberLine.toString());
                }
                if (newLine.contains("class_name"))
                {
                    QStringRef numberLine(&newLine,77,1);
                    classType = numberLine.toInt() - 1;
                }
            }
            else
                break;
        }
        //if no size, then no cards gotten
        if (deckVector.size() == 0)
        {
            QMessageBox::StandardButton errorMessage;
            errorMessage = QMessageBox::information(this, tr(""),
                                             tr("No cards found. Make sure you're not in the deckbuilder."));
            emit signalGenerateDeck();
            return;
        }

        // Create a deck from the loaded values
        //for sanity's sake, clear the deck first
        playingDeck.clear();
        playingDeck.setClass(classType);
        DeckNameEdit->setText("");

        int pageAmount = deckVector.size()/2;

        for (int i = 0; i < pageAmount; i++)
        {

            int cardAmount = deckVector[i*2].toInt();
            int cardID = deckVector[i*2+1].toInt();
            for(int j = 0; j < cardAmount; j++)
                playingDeck.addCard(cardID);
        }
        loadDeck(model);
        emit signalGenerateDeck();
    }
}

void frmWindow::closeEvent(QCloseEvent *event)
{
    if (needSave)
    {
        //do some slot
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Unsaved Deck"), tr("The deck is unsaved. Do you want to save?"),
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            slotSave();
        }
    }
    QFile file(dir.absolutePath() + "/data/settings.ini");
    if (!file.open(QIODevice::WriteOnly)){
       QMessageBox::StandardButton errorMessage;
       errorMessage = QMessageBox::information(this, tr(""),
                                        tr("Couldn't save settings"));

       file.close();
       PlayingDeckList->close();
       event->accept();
       return;
    }

    QTextStream out(&file);
    for (auto e: settingsMap.toStdMap()){
        out << e.first << '=' << e.second << "\r\n";
    }

    file.close();
    PlayingDeckList->close();
    event->accept();
}
