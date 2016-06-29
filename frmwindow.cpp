#include "frmwindow.h"
#include "ui_frmwindow.h"
#include "asmopencv.h"
#include "perceptualhash.h"
#include "cardlist.h"

#include <QtWinExtras/QtWin>
#include <QPixmap>
#include <QDirIterator>

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

        QDirIterator it(NewPath, QStringList() << "*.png", QDir::Files, QDirIterator::Subdirectories);

        std::vector<QString> filenames;
        std::vector<ulong64> filephash;

        while (it.hasNext()) {
            it.next();
            filenames.push_back(it.fileName());
            cv::Mat temp = cv::imread(it.filePath().toStdString());
            ulong64 somelong = PerceptualHash::phash(temp);
            filephash.push_back(somelong);
            currentDeck.addCard("test",it.filePath().toStdString(),somelong);
        };

        setWindowTitle(QString::number(currentDeck.deckPHash.size()));
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
        //Create a "you start" mat
        matTexture = cv::imread("pic.png");
        matTexturePhash = PerceptualHash::phash(matTexture);



        if (!matTexture.data)
        {
            setWindowTitle("hi");
        }

        ignoreNext = 0;

        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(update()));
        timer->start(50);
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
        boxRect.setRect(boxLeft,boxTop,boxWidth,boxHeight);
        drawer = pixmap.copy(boxRect);
        mat = ASM::QPixmapToCvMat(drawer);
        imagePHash = PerceptualHash::phash(mat);

        distance = PerceptualHash::hammingDistance(matTexturePhash, imagePHash);

        if (distance < 15 && ignoreNext < 1)
        {
            counter++;
            ignoreNext = 30;
            setWindowTitle( QString::number(counter));
            curState = Ui::STATE::FINDCARD;
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
        input[0] = cv::Point2f(0.0422 * cardWidth, cardHeight);
        input[1] = cv::Point2f(0.0129 * cardWidth,0.0131 * cardHeight);
        input[2] = cv::Point2f(0.88 * cardWidth,0.00526 * cardHeight);
        input[3] = cv::Point2f(0.984 * cardWidth,0.982 * cardHeight);

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
        PerceptualHash::ComparisonResult bestGuess = PerceptualHash::best(imagePHash, currentDeck.deckPHash);

        setWindowTitle(QString::number(bestGuess.distance));

        // TODO: Get a better Algorithm
        if (bestGuess.distance < 20 && ignoreNext < 1)
        {
            cardlist::Card best = currentDeck.cardsInDeck[bestGuess.index];
            cv::imwrite("guessed.png", resultMat);
            setWindowTitle(QString::fromStdString(best.filename));
            curState = Ui::STATE::MYTURN;
        }

        break;
    }

    ui->miniscreen->setPixmap(drawer);

}

void frmWindow::on_pushButton_clicked()
{
    cv::imwrite("return.png", mat);
}
