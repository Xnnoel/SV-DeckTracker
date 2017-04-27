#include "frmwindow.h"
#include "ui_frmwindow.h"

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
#include <algorithm>
#include <iostream>

#pragma comment(lib, "user32.lib")

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
    last = false;

    // Set up some inits
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

    connect(delegate, SIGNAL(downClicked(int)), model, SLOT(slotDown(int)));
    connect(delegate, SIGNAL(upClicked(int)), model, SLOT(slotUp(int)));
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

    // setup from start slot
    setWindowTitle("Shadowverse Deck Tracker");

    // set blinkers to 0
    memset(blinker, 0 , sizeof(blinker));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(30);

    //Remove edit button
    mainLayout->removeWidget(editButton);
    editButton->setGeometry(0,0,0,0);

    //replace blank space with turn log
    mainLayout->addWidget(turnLog,5,0);
    mainLayout->removeWidget(labelBlank);
    labelBlank->setGeometry(0,0,0,0);
    turnLog->clear();
    turnLog->append("Finding window\n******************\n");

    loadDeck(model);
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
            playingDeck.cardsInDeck[j] = playingDeck.cardsInDeck[j-1];
            playingDeck.countInDeck[j] = playingDeck.countInDeck[j-1];
            playingDeck.cardsInDeck[j-1] = tempCard;
            playingDeck.countInDeck[j-1] = tempCount;
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
    /// In the update function, we will
    /// guess what state the program is in.
    bool valid = pr.isValid();
    std::vector<int> nums = pr.update();

    if (valid)
    {
        if (last != valid)
        {
            // do initial set up here
            // reset log? reset list
            turnLog->append("******************\nStarting Game\n******************\n");
            memset(blinker, 0 , sizeof(blinker));
            prevHand.clear();
            loadDeck(model);
        }

        // compare to prev hand
        int indexA = 0,indexB = 0;
        std::vector<int> playedCards;
        while (indexA < prevHand.size() && indexB < nums.size())
        {
            if (prevHand[indexA] == nums[indexB])
                indexB++;
            else
                playedCards.push_back(prevHand[indexA]);
            indexA++;
        }

        //Played cards
        int i;
        for (i = indexA; i < prevHand.size();++i)
        {
            playedCards.push_back(prevHand[i]);
        }

        // For any cards played this turn
        for (i = 0; i < playedCards.size(); i++)
        {
            model->blink(playingDeck.getPosition(playedCards[i]));
        }

        // Cards added, all the last ones
        for (i = indexB; i < nums.size();++i)
        {
            if (playingDeck.cardExists(nums[i]))
            {
                int rownum = playingDeck.getPosition(nums[i]);
                model->subCard(nums[i]);
                blinker[rownum] = 40;
                delegate->blinkEffect(rownum,0);
            }
            QString cardname = cardDatabase.getCard(nums[i]).name;
            turnLog->append("Drew " + cardname + '\n');
        }

        // blinker effect
        for (i = 0; i < playingDeck.getDeckSize(); i++)
        {
            if (blinker[i] > 0)
            {
                blinker[i]--;
                delegate->blinkEffect(i, blinker[i]);
                model->blink(i);
            }
        }

        prevHand = nums;

        delegate->setCardsInHand(nums);
    }
    else
    {
        // If we reach here then game has ended
        if (last != valid)
        {
            turnLog->append("******************\nGame Ended\n******************\n");
            prevHand.clear();
            loadDeck(model);
        }
    }

    last = valid;

    // TODO: Fix counds bounced back counting as cards drawn

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

    ClearDeckAction = new QAction(tr("&Clear Loaded Deck"), this);
    connect(ClearDeckAction, &QAction::triggered, this, &frmWindow::slotClearDeck);

    LoadAction = new QAction(tr("&Load from file"), this);
    LoadAction->setToolTip(tr("Load a deck"));
    connect(LoadAction, &QAction::triggered, this, &frmWindow::slotLoad);

    LoadURLAction = new QAction(tr("&Load using deck code"), this);
    LoadURLAction->setToolTip(tr("Load using shadowportal URL"));
    connect(LoadURLAction, &QAction::triggered, this, &frmWindow::slotLoadURL);

    SaveAsAction = new QAction(tr("&Save deck as..."), this);
    SaveAsAction->setToolTip(tr("Save current deck as"));
    connect(SaveAsAction, &QAction::triggered, this, &frmWindow::slotSaveAs);

    SaveAction = new QAction(tr("&Save deck"), this);
    SaveAction->setToolTip(tr("Save current deck"));
    SaveAction->setShortcuts(QKeySequence::Save);
    connect(SaveAction, &QAction::triggered, this, &frmWindow::slotSave);

    SetBaseAction = new QAction(tr("&Set base address"), this);
    SaveAction->setToolTip(tr("Set base address in hex"));
    connect(SetBaseAction, &QAction::triggered, this, &frmWindow::slotSetBase);

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
    DeckMenu->addAction(ClearDeckAction);
    DeckMenu->addSeparator();
    DeckMenu->addAction(SaveAction);
    DeckMenu->addAction(SaveAsAction);

    HelpMenu = new Menu();
    HelpMenu->setTitle(tr("&Help"));
    menuBar()->addMenu(HelpMenu);
    HelpMenu->addAction(HelpAction);
    HelpMenu->addSeparator();
    HelpMenu->addAction(About);
    HelpMenu->addAction(SetBaseAction);
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

void frmWindow::slotClearDeck()
{
    playingDeck.clear();
    loadDeck(model);
    needSave = false;
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
        tr("Open Deck"), dir.absolutePath() + "/Decks/NewDeck.dck", tr("Image Files (*.dck)"));
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
                                    tr("Deck code:"), QLineEdit::Normal,
                                    "", &ok,Qt::WindowSystemMenuHint | Qt::WindowTitleHint);

    if (ok && !text.isEmpty())
    {
        if (text.size() == 4)
        {
            // probably a deck code
            QNetworkAccessManager *managertemp = new QNetworkAccessManager(this);
            connect(managertemp, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyCodeFinished(QNetworkReply*)));
            QString texttwo = "https://shadowverse-portal.com/api/v1/deck/import?format=json&deck_code=" + text;
            QUrl urltemp(texttwo);
            QNetworkRequest reqTemp(urltemp);
            managertemp->get(reqTemp);
        }
        else
        {
            QMessageBox::StandardButton errorMessage;
            errorMessage = QMessageBox::information(this, tr(""),
                                             tr("An error has occured. Please provide a 4 character code."));
        }
    }
    needSave = true;
}

void frmWindow::replyCodeFinished(QNetworkReply* reply)
{
    if(reply->error() == QNetworkReply::NoError) {
            QString strReply = (QString)reply->readAll();

            QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());

            QJsonObject jsonObject = jsonResponse.object();

            QJsonObject jsonData = jsonObject["data"].toObject();

            QString hashcode = jsonData["hash"].toString();

            QString text = "https://shadowverse-portal.com/deck/" + hashcode;

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

        } else {
            qDebug() << "ERROR";
        }

        delete reply;
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
                                        "Version 0.8.6\n"
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

    editButton = new QPushButton("Edit");
    connect(editButton, SIGNAL(released()), this, SLOT(slotEditMode()));

    mainLayout->addWidget(labelDeckName, 0, 0);
    mainLayout->addWidget(DeckNameEdit, 1, 0);
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
    editButton->setGeometry(0,0,0,0);
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

        for (int i = 403; i < 414; i++)
            editmodel->addCard(cardDatabase.cardID[i]);

        for (int i = 512; i < 526; i++)
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
            for (int i = 414; i < 428; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 526; i < 539; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        case 1: //royal
            for (int i = 19; i < 30; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 151; i < 193; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 428; i < 442; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 539; i < 552; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        case 2: //witch
            for (int i = 30; i < 41; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 193; i < 235; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 442; i < 456; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 552; i < 565; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        case 3: //dragon
            for (int i = 41; i < 52; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 235; i < 277; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 456; i < 470; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 565; i < 578; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        case 4: //necro
            for (int i = 52; i < 63; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 277; i < 319; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 470; i < 484; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 578; i < 591; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        case 5: //blood
            for (int i = 63; i < 74; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 319; i < 361; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 484; i < 498; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 591; i < 604; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
        break;
        case 6: //bishop
            for (int i = 74; i < 85; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 361; i < 403; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 498; i < 512; i++)
                editmodel->addCard(cardDatabase.cardID[i]);
            for (int i = 604; i < 617; i++)
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

void frmWindow::slotSetBase()
{
    // create a window to take in a string

    QString text = QInputDialog::getText(this, tr("Set Base"),
                                         tr("Hex:"), QLineEdit::Normal,
                                         tr("00000000"));
    int base = text.toInt(nullptr, 16);
    pr.setBase(base);
}

void frmWindow::replyFinished(QNetworkReply * reply)
{
    if ( reply->error() != QNetworkReply::NoError ) {
        QMessageBox::StandardButton errorMessage;
        errorMessage = QMessageBox::information(this, tr(""),
                                         tr("An error has occured. Please provide a proper code."));
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


                if (newLine.contains("2pick"))
                {
                    //setWindowTitle("2pick");
                }

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
                                             tr("No cards found. Make sure you're providing a proper code."));
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
    delete reply;
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
