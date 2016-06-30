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
    currentDeck("testname")
{
    ui->setupUi(this);

    // verify bluestacks is open and set up a window handle to the screen
    handle = 0;
    handle = ::FindWindow(NULL, L"BlueStacks App Player");

    mat = 0;
    matTexture = 0;


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

        //create CV map
        QString NewPath = dir.absolutePath() + "/Images/";

        setWindowTitle("HI");

        /// This generates a hash table from images in the folder x and saves it somewhere
        QString filename = NewPath + "names.txt";
        QFile file( filename );

        std::vector<QString> filenames;
        std::vector<ulong64> filephash;
        std::vector<QString> fileid;


        if ( file.open(QIODevice::ReadOnly) )
        {
            QTextStream in( &file );
            QDirIterator it(NewPath, QStringList() << "*.png", QDir::Files, QDirIterator::Subdirectories);

            while (it.hasNext()) {
                it.next();
                fileid.push_back(in.readLine());
                filenames.push_back(it.fileName().left(9));
                cv::Mat temp = cv::imread(it.filePath().toStdString());
                ulong64 somelong = PerceptualHash::phash(temp);
                filephash.push_back(somelong);
            };
        }
        file.close();

        //Serialize into JSON
        QFile saveFile("save.json");
        if (!saveFile.open(QIODevice::WriteOnly)) {
           qWarning("Couldn't open save file.");
       }
        QJsonObject gameObject;
        //fill stuff in here
        QJsonArray cardArray;

        for (int i = 0; i < filenames.size(); i++)
        {
            QJsonObject card;

            unsigned int low32bits = filephash[i] & 0x00000000ffffffff;
            unsigned int high32bits = filephash[i] >> 32;
            QString PHashJson = QString::number(high32bits) + QString::number(low32bits);
            card["ID"] = filenames[i];
            card["Cost"] = 1;
            card["pHash"] = PHashJson;
            card["Name"] = fileid[i];

            cardArray.append(card);
        }
        gameObject["Cards"] = cardArray;

        QJsonDocument saveDoc(gameObject);
        saveFile.write(saveDoc.toJson());
        saveFile.close();

        setWindowTitle(QString::number(filephash.size()));
        /*
        QString filename="Listofnames.txt";

        QFile file( filename );
        if ( file.open(QIODevice::ReadWrite) )
        {
            QTextStream stream( &file );
            for(int i = 0; i < filenames.size(); i++)
            {
                stream << filenames[i] << "=";
                unsigned int low32bits = filephash[i] & 0x00000000ffffffff;
                unsigned int high32bits = filephash[i] >> 32;

                stream << high32bits << "=" << low32bits << endl;
            }
        }
        file.close();
        */

        /// Load in a new deck
        /*
        QString filename="Listofnames.txt";

        QFile file( filename );

        if ( file.open(QIODevice::ReadOnly) )
        {
            QTextStream in( &file );
               while (!in.atEnd())
               {
                   QStringList wordFileList;
                    QString line = in.readLine();
                    if (!line.isEmpty())
                    {
                        wordFileList = line.split("=");
                    }
                    ulong64 cardPHash= wordFileList[1].toInt();
                    cardPHash << 32;
                    cardPHash += wordFileList[2].toInt();

                    currentDeck.addCard("test", wordFileList[0].toStdString(), cardPHash);
                    setWindowTitle(wordFileList[2]);
               }

        }
        file.close();
        */

        savedata();

        /*

        //Create a "you start" mat
        matTexture = cv::imread("pic.png");
        matTexturePhash = PerceptualHash::phash(matTexture);

        ignoreNext = 0;

        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(update()));
        timer->start(20);

        */
    }
}

frmWindow::~frmWindow()
{
    delete ui;
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
        if (toyou > 0)
            toyou--;
        boxRect.setRect(boxLeft,boxTop,boxWidth,boxHeight);
        drawer = pixmap.copy(boxRect);
        mat = ASM::QPixmapToCvMat(drawer);
        imagePHash = PerceptualHash::phash(mat);

        distance = PerceptualHash::hammingDistance(matTexturePhash, imagePHash);

        setWindowTitle("looking for myturn" + QString::number(distance));

        if (distance < 20 && ignoreNext < 1)
        {
            if (toyou == 0)
            {
                toyou = 2;
            }
            else
            {
                turncounter++;
                ignoreNext = 20;
                QString trn = QString::number(turncounter);
                ui->pushButton->setText(trn);
                curState = Ui::STATE::FINDCARD;
                toyou = 0;
            }
        }

        //DEBUG PURPOSES

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
        input[2] = cv::Point2f(0.795f * cardWidth,0.1763f * cardHeight);
        input[3] = cv::Point2f(0.8668f * cardWidth,0.8763f * cardHeight);

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
            if (bestguesses[i].distance < 20 )
            {
                int val = 20 - bestguesses[i].distance;
                pass = true;
                for (int j = 0; j < names.size(); j++)
                    if (names[j] == currentDeck.cardsInDeck[bestguesses[i].index].filename)
                    {
                        count[j] += val;
                        continue;
                    }
                names.push_back( currentDeck.cardsInDeck[bestguesses[i].index].filename);
                count.push_back(val);
            }
        }

        if (ignoreNext > 0)
            setWindowTitle("ignoring");
        else
            setWindowTitle(QString::number( bestguesses[0].distance));

        if ((pass && ignoreNext < 1) || counter > 0)
        {
            if (counter < 4)
            {
            ui->tableWidget->setItem(counter * 3 + 0,0,new QTableWidgetItem(QString::number( bestguesses[0].distance)));
            ui->tableWidget->setItem(counter * 3 + 1,0,new QTableWidgetItem(QString::number( bestguesses[1].distance)));
            ui->tableWidget->setItem(counter * 3 + 2,0,new QTableWidgetItem(QString::number( bestguesses[2].distance)));
            ui->tableWidget->setItem(counter * 3 + 0,1,new QTableWidgetItem(QString::fromStdString(currentDeck.cardsInDeck[bestguesses[0].index].filename.substr(89,9))));
            ui->tableWidget->setItem(counter * 3 + 1,1,new QTableWidgetItem(QString::fromStdString(currentDeck.cardsInDeck[bestguesses[1].index].filename.substr(89,9))));
            ui->tableWidget->setItem(counter * 3 + 2,1,new QTableWidgetItem(QString::fromStdString(currentDeck.cardsInDeck[bestguesses[2].index].filename.substr(89,9))));

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
                drawme.load(QString::fromStdString( names[index]));
                names.clear();
                count.clear();
                ui->Image->setPixmap(drawme);
            }
        }

        break;
    }
}

void frmWindow::on_pushButton_clicked()
{
    turncounter = 0;
}

void frmWindow::savedata()
{





}
