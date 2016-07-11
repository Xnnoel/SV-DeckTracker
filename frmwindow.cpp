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

#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>

#include <qtimer.h>
#include <Windows.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

Q_GUI_EXPORT QPixmap qt_pixmapFromWinHBITMAP(HBITMAP bitmap, int hbitmapFormat=0);

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Gdi32.lib")

#define WINWIDTH 550
#define EDITWINWIDTH 850
#define MAXWINHEIGHT 810

std::wstring s2ws(const std::string& s);

QPushButton* editButton;

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
    setWindowTitle("Shadowverse Deck Tracker");

    //add menubar?
    this->setMyLayout();
    createActions();
    createMenus();

    ///DEBUG
    /// ADD IN SOME SAMPLE CARDS FOR CURRENT DECK
    ///IDEALLY WE LOAD IN PLAYINGDECK, THEN COPY INTO MODEL
    playingDeck.setName("Midrange Royal?");
    playingDeck.setDesc("This is a description of the deck. Hopefully you can write tips and ideas here on what to evolve or something. I dunno.");
    playingDeck.addCard(100211010);
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
    delegate->editMode = false;
    playingDeck.setClass(1);


    // Set up model view
    model = new SVListModel;
    model->setPointer(&cardDatabase, &playingDeck);
    delegate = new CardDelegate;
    delegate->setPointers(&cardDatabase, &playingDeck);

    connect(delegate, SIGNAL(upClicked(int)), model, SLOT(slotUp(int)));
    connect(delegate, SIGNAL(downClicked(int)), model, SLOT(slotDown(int)));

    connect(model, SIGNAL(countChanged(int)), this, SLOT(updateCount(int)));


    PlayingDeckList->setModel(model);
    PlayingDeckList->setItemDelegate(delegate);
    loadDeck(model);        //load data into model
    PlayingDeckList->show();

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


    //// DEBUG ADD WINDOW TO SIDE




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
    setFixedWidth(WINWIDTH);
    int listsize = playingDeck.cardsInDeck.size() * 35 + 4;
    PlayingDeckList->setFixedHeight(std::min(MAXWINHEIGHT-71, std::max(listsize,400)));
    setFixedHeight(std::min(MAXWINHEIGHT, PlayingDeckList->height() + 70));
    //Set text description
    DeckNameEdit->setText(QString::fromStdString(playingDeck.getName()));
    DeckDescEdit->document()->setPlainText(QString::fromStdString(playingDeck.getDescription()));

    int decksize = playingDeck.getDeckSize();

    updateCount(decksize);

}

void frmWindow::updateCount(int cardsize)
{
    int decksize = playingDeck.getDeckSize();

    QString cardCountLabel = QString::number(cardsize) + "/" + QString::number(decksize) + " Cards";
    label4->setText(cardCountLabel);
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
        setWindowTitle("Can't Find Window...");
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
    DeckMenu->setTitle(tr("&Deck"));
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
    playingDeck.clear();
    playingDeck.setClass(0);
    loadDeck(model);
    createEditor(); //spawns an editor on the right side
    slotLoadEdit(0);
}

void frmWindow::slotRoyal()
{
    playingDeck.clear();
    playingDeck.setClass(1);
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
}
void frmWindow::slotWitch()
{
    playingDeck.clear();
    playingDeck.setClass(2);
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
}
void frmWindow::slotDragon()
{
    playingDeck.clear();
    playingDeck.setClass(3);
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
}
void frmWindow::slotNecro()
{
    playingDeck.clear();
    playingDeck.setClass(4);
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
}
void frmWindow::slotVampire()
{
    playingDeck.clear();
    playingDeck.setClass(5);
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
}
void frmWindow::slotBishop()
{
    playingDeck.clear();
    playingDeck.setClass(6);
    loadDeck(model);
    createEditor();
    slotLoadEdit(0);
}
void frmWindow::slotLoad()
{

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Save Deck as"), dir.absolutePath() + "/Decks/NewDeck", tr("Image Files (*.dck)"));
    QFile loadfile(fileName);
    if (!loadfile.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open");
        return;
    }
    QTextStream textStream(&loadfile);
    QString line = textStream.readLine();
    std::string deckname = line.toStdString();
    playingDeck.clear();
    line = textStream.readLine();
    int classnum = std::stoi(line.toStdString());
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
    playingDeck.setClass(classnum);
    loadfile.close();
    loadDeck(model);
    delegate->editMode = false;
    mainLayout->removeWidget(EditDeckList);
    mainLayout->removeWidget(okButton);
    mainLayout->removeWidget(neutralBox);
    mainLayout->removeWidget(classBox);
    EditDeckList->setGeometry(0,0,0,0);
    okButton->setGeometry(0,0,0,0);
    neutralBox->setGeometry(0,0,0,0);
    classBox->setGeometry(0,0,0,0);
    setFixedWidth(WINWIDTH);
}
void frmWindow::slotSave()
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

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Deck as"), dir.absolutePath() + "/Decks/NewDeck", tr("Decks (*.dck)"));
    QFile savefile(fileName);
    if (!savefile.open(QIODevice::WriteOnly)) {
           qWarning("Couldn't open save");
           return;
    }

    QString textname = DeckNameEdit->text();
    playingDeck.setName(textname.toStdString());
    QString textdesc = DeckDescEdit->document()->toPlainText();
    playingDeck.setDesc(textdesc.toStdString());

    std::string final = playingDeck.getName() + '\n';
    savefile.write(final.c_str());
    std::string classnum = std::to_string(playingDeck.getClass()) + '\n';
    savefile.write(classnum.c_str());
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

void frmWindow::setMyLayout()
{


    //layout of all those things yay
    mainLayout = new QGridLayout();
    label1 = new QLabel("Deck Name");
    label1->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    label1->setMaximumHeight(20);

    label2 = new QLabel("Deck Description");
    label2->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    label2->setMaximumHeight(20);

    label3 = new QLabel("");
    label3->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    label3->setMinimumHeight(200);

    label4 = new QLabel("Cards");
    label4->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    label4->setMaximumHeight(20);
    label4->setAutoFillBackground(true);

    DeckNameEdit = new QLineEdit();
    DeckNameEdit->setMaximumHeight(25);
    DeckNameEdit->setMaxLength(35);
    DeckNameEdit->setPlaceholderText(tr(" Deck name here"));

    DeckDescEdit = new QPlainTextEdit();
    DeckDescEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    DeckDescEdit->setMinimumHeight(250);
    DeckDescEdit->setPlaceholderText(tr(" Default Description"));

    PlayingDeckList = new QListView();
    PlayingDeckList->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    PlayingDeckList->setFixedWidth(270);

    startButton = new QPushButton("Start");
    editButton = new QPushButton("Edit");

    mainLayout->addWidget(label1, 0, 0);
    mainLayout->addWidget(DeckNameEdit, 1, 0);
    mainLayout->addWidget(label2, 2, 0);
    mainLayout->addWidget(DeckDescEdit, 3, 0);
    mainLayout->addWidget(startButton,4,0);
    mainLayout->addWidget(label3, 5, 0);
    mainLayout->addWidget(label4, 0, 1);
    mainLayout->addWidget(editButton, 0, 2);
    mainLayout->addWidget(PlayingDeckList, 1, 1,5,2, Qt::AlignTop);

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

}

void frmWindow::createEditor()
{
    // This Spawns an editor to the right side that you can use to click on the deck with
    // Need a new list delegate and model for the editor damnit
    // shrink window size
    // Maybe we cheat and have it loaded beforehand so we only need to expand win width?
    mainLayout->addWidget(okButton, 0, 3);
    mainLayout->addWidget(neutralBox, 0, 4);
    mainLayout->addWidget(classBox, 0, 5);
    mainLayout->addWidget(EditDeckList, 1, 3,5,3);
    neutralBox->setChecked(true);
    classBox->setChecked(true);

    setFixedWidth(EDITWINWIDTH);

    delegate->editMode = true;
    PlayingDeckList->setFixedHeight(MAXWINHEIGHT-68);
    setFixedHeight(MAXWINHEIGHT);
}

void frmWindow::slotButtonPushed()
{
    mainLayout->removeWidget(EditDeckList);
    mainLayout->removeWidget(okButton);
    mainLayout->removeWidget(neutralBox);
    mainLayout->removeWidget(classBox);
    EditDeckList->setGeometry(0,0,0,0);
    okButton->setGeometry(0,0,0,0);
    neutralBox->setGeometry(0,0,0,0);
    classBox->setGeometry(0,0,0,0);
    setFixedWidth(WINWIDTH);
    delegate->editMode = false;
    loadDeck(model);
}

void frmWindow::slotLoadEdit(int a)
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
    qWarning("refreshed list");
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
    qWarning("updatehtecount");
}
