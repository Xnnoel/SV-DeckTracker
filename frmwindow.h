#ifndef FRMWINDOW_H
#define FRMWINDOW_H

#include <QMainWindow>

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

namespace Ui {
class frmWindow;
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
    void on_pushButton_clicked();

private:
    Ui::frmWindow *ui;

    cv::Mat matTexture;
    cv::Mat mat;

    HDC hdcScreen;
    HDC hdc;
    HBITMAP hbmp;
    HWND handle;
    int counter;

    int height;
    int width;
    int left;
    int top;

    int boxHeight;
    int boxWidth;
    int boxLeft;
    int boxTop;
};

#endif // FRMWINDOW_H
