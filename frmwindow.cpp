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

#include <qtimer.h>
#include <Windows.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

Q_GUI_EXPORT QPixmap qt_pixmapFromWinHBITMAP(HBITMAP bitmap, int hbitmapFormat=0);

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Gdi32.lib")

frmWindow::frmWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmWindow),
    dir("."),
    currentDeck("current", &cardDatabase),
    playingDeck("Don't use me", &cardDatabase)
{
    ui->setupUi(this);

    // verify bluestacks is open and set up a window handle to the screen
    handle = 0;
    handle = ::FindWindow(NULL, L"BlueStacks App Player");

    mat = 0;
    matTexture = 0;

    ///DEBUG
    /// ADD IN SOME SAMPLE CARDS FOR CURRENT DECK
    ///IDEALLY WE LOAD IN PLAYINGDECK, THEN COPY INTO CURRENT
    currentDeck.addCard(100211010);
    currentDeck.addCard(101211120);
    currentDeck.addCard(100211020);
    currentDeck.addCard(100211030);
    currentDeck.addCard(100211040);
    currentDeck.addCard(100214010);
    currentDeck.addCard(100221010);
    currentDeck.addCard(100221020);
    currentDeck.addCard(101211060);
    currentDeck.addCard(101211070);
    currentDeck.addCard(101211090);
    currentDeck.addCard(101211110);
    currentDeck.addCard(101221010);
    currentDeck.addCard(101221070);
    currentDeck.addCard(101221100);
    currentDeck.addCard(101231040);
    currentDeck.addCard(101234020);
    currentDeck.addCard(101241020);
    currentDeck.addCard(101241030);
    currentDeck.addCard(101031020);

    QListView *list =  ui->listView;
    QStandardItemModel *model = new QStandardItemModel();
    CardDelegate * delegate = new CardDelegate(&cardDatabase);

    list->setItemDelegate(delegate);
    list->setModel(model);
    loadDeck(model);
    list->show();

    if (handle != 0)
    {
        // Start the update loop to check for cards in images
        counter = 0;
        turncounter = 0;

        //Window rect
        RECT rc;
        GetClientRect(handle, &rc);
        width = (rc.right - rc.left)-65;
        height = (rc.bottom - rc.top)-43;
        top = rc.top+43;
        left = rc.left+65;

        boxLeft = (int)round(0.261 * width + left);
        boxTop = (int)round(0.341 * height + top);
        boxWidth = (int)round(0.4735 * width);
        boxHeight = (int)round(0.1965 * height);


        cardLeft = (int)round(0.5699 * width + left);
        cardTop = (int)round(0.4157 * height + top);
        cardWidth = (int)round(0.210 * width);
        cardHeight = (int)round(0.4606 * height);


        //create bitmap and screen to save rect
        hdcScreen = GetDC(NULL);
        hdc = CreateCompatibleDC(hdcScreen);
        hbmp = CreateCompatibleBitmap(hdcScreen,
            width , height);
        SelectObject(hdc, hbmp);

        //Guess the current state of the image
        curState = Ui::STATE::MYTURN;

        //Create a "you start" mat
        matTexture = cv::imread("pic.png");
        matTexturePhash = PerceptualHash::phash(matTexture);

        ignoreNext = 0;

        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(update()));
        timer->start(10);
        //
    }
}

frmWindow::~frmWindow()
{
    delete ui;
}

void frmWindow::loadDeck(QStandardItemModel* model)
{
    for (int i = 0; i < currentDeck.cardsInDeck.size(); i++)
    {
        Card card = cardDatabase.getCard( currentDeck.cardsInDeck[i]);
        QStandardItem *item = new QStandardItem();
        item->setData(card.manaCost,CardDelegate::Cost);
        item->setData(card.ID,CardDelegate::ID);
        item->setData(card.name,CardDelegate::Name);
        item->setData(currentDeck.countInDeck[i],CardDelegate::Amount);
        item->setEditable(false);
        model->appendRow(item);
    }
}

void frmWindow::update()
{
    /// In the update function, we will continous loop to try
    /// and guess what state the program is in. Mainly want to
    /// see if a game had started or not.
    //decrement all if exist
    if (ignoreNext > 0)ignoreNext--;


    //otherwise check for card
    PrintWindow(handle, hdc, PW_CLIENTONLY);

    QPixmap pixmap = qt_pixmapFromWinHBITMAP(hbmp);

    QRect boxRect;
    QPixmap drawer;
    ulong64 imagePHash;

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

        if (distance < 15 && ignoreNext < 1)
        {
            cv::imwrite("new_round.png", mat);
            turncounter++;
            ignoreNext = 30;
            QString trn = QString::number(turncounter);
            ui->pushButton->setText(trn);
            curState = Ui::STATE::FINDCARD;
        }

        break;
    case Ui::STATE::FINDCARD:
        boxRect.setRect(cardLeft,cardTop,cardWidth,cardHeight);
        drawer = pixmap.copy(boxRect);
        mat = ASM::QPixmapToCvMat(drawer);

        //Perspective shift on card for better readability (why are they tilted)
        cv::Point2f input[4];
        cv::Point2f output[4];
\
        //in case of window size changed, use percentages
        input[0] = cv::Point2f(0.172f * cardWidth, 0.871f * cardHeight);
        input[1] = cv::Point2f(0.1558f * cardWidth,0.171f * cardHeight);
        input[2] = cv::Point2f(0.785f * cardWidth,0.1763f * cardHeight);
        input[3] = cv::Point2f(0.8568f * cardWidth,0.8763f * cardHeight);

        //doesnt matter here, so long as output size fits here
        output[0] = cv::Point2f(0,380);
        output[1] = cv::Point2f(0,0);
        output[2] = cv::Point2f(297,0);
        output[3] = cv::Point2f(297,380);

        cv::Mat lambda( 2, 4, CV_32FC1 );
        lambda = cv::Mat::zeros( mat.rows, mat.cols, mat.type() );

        lambda = cv::getPerspectiveTransform(input,output);
        cv::warpPerspective(mat, resultMat, lambda, cv::Size(297,380));

        imagePHash = PerceptualHash::phash(resultMat);

        std::vector<PerceptualHash::ComparisonResult> bestguesses = PerceptualHash::nbest(3,imagePHash, currentDeck.deckPHash);

        bool pass = false;

        for (int i = 0; i < 3; i++)
        {
            if (bestguesses[i].distance <= 20 )
            {
                int val = 20 - bestguesses[i].distance + 1;
                pass = true;
                for (int j = 0; j < id.size(); j++)
                    if (id[j] == currentDeck.cardsInDeck[bestguesses[i].index])// FIX
                    {
                        count[j] += val;
                        continue;
                    }
                id.push_back( currentDeck.cardsInDeck[bestguesses[i].index]);
                count.push_back(val);
            }
        }

        if (ignoreNext > 0)
            setWindowTitle("ignoring");
        else
            setWindowTitle(QString::number( bestguesses[0].distance));

        if ((pass && ignoreNext < 1) || counter > 0)
        {
            if (counter < 8)
            {
            ui->tableWidget->setItem(counter * 3 + 0,0,new QTableWidgetItem(QString::number( bestguesses[0].distance)));
            ui->tableWidget->setItem(counter * 3 + 1,0,new QTableWidgetItem(QString::number( bestguesses[1].distance)));
            ui->tableWidget->setItem(counter * 3 + 2,0,new QTableWidgetItem(QString::number( bestguesses[2].distance)));
            ui->tableWidget->setItem(counter * 3 + 0,1,new QTableWidgetItem(cardDatabase.getCard(currentDeck.cardsInDeck[bestguesses[0].index]).name));
            ui->tableWidget->setItem(counter * 3 + 1,1,new QTableWidgetItem(cardDatabase.getCard(currentDeck.cardsInDeck[bestguesses[1].index]).name));
            ui->tableWidget->setItem(counter * 3 + 2,1,new QTableWidgetItem(cardDatabase.getCard(currentDeck.cardsInDeck[bestguesses[2].index]).name));

            std::string filetitledraw = std::to_string(counter) + "guessed.png";

            cv::imwrite(filetitledraw, resultMat);
            counter ++;
            }
            else
            {
                counter = 0;
                curState = Ui::STATE::MYTURN;

                int max = 0;
                int index = 0;
                for (int i = 0; i < count.size(); i++)
                {
                    if (count[i] > max)
                    {
                        max = count[i];
                        index = i;
                    }
                }


                QPixmap drawme;
                std::string temp = "Images";
                temp += id[index];
                temp += ".png";
                drawme.load(QString::fromStdString( temp));

                setWindowTitle(cardDatabase.getCard(id[index]).name);

                id.clear();
                count.clear();

            }
        }

        break;
    }
}

void frmWindow::on_pushButton_clicked()
{
    turncounter = 0;
}

