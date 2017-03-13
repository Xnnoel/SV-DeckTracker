#include "frmwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::addLibraryPath("./");
    QApplication a(argc, argv);
    frmWindow w;
    w.show();

    return a.exec();
}
