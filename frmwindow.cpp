#include "frmwindow.h"
#include "ui_frmwindow.h"
#include "asmopencv.h"
#include "perceptualhash.h"

#include <QtWinExtras/QtWin>
#include <QPixmap>
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
    ui(new Ui::frmWindow)
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

        boxLeft = (int)floor(0.261 * width + left);
        boxTop = (int)floor(0.341 * height + top);
        boxWidth = (int)floor(0.4735 * width);
        boxHeight = (int)floor(0.1965 * height);

        //create bitmap and screen to save rect
        hdcScreen = GetDC(NULL);
        hdc = CreateCompatibleDC(hdcScreen);
        hbmp = CreateCompatibleBitmap(hdcScreen,
            width , height);
        SelectObject(hdc, hbmp);

        //Guess the current state of the image
        curState = Ui::STATE::TITLE;

        //create CV map
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
    switch (curState)
    {
    default:
        //Print to memory hdc
        PrintWindow(handle, hdc, PW_CLIENTONLY);

        QPixmap pixmap = qt_pixmapFromWinHBITMAP(hbmp);

        QRect boxRect(boxLeft,boxTop,boxWidth,boxHeight);
        QPixmap drawer = pixmap.copy(boxRect);

        mat = ASM::QPixmapToCvMat(drawer);
        ulong64 imagePHash = PerceptualHash::phash(mat);
        int distance = PerceptualHash::hammingDistance(matTexturePhash, imagePHash);

        ui->miniscreen->setPixmap(drawer);

        if (ignoreNext > 0)ignoreNext--;
        if (distance < 15 && ignoreNext < 1)
        {
            counter++;
            ignoreNext = 60;
            setWindowTitle( QString::number(counter));
        }
    }
}

void frmWindow::on_pushButton_clicked()
{
    cv::imwrite("return.png", mat);
}
