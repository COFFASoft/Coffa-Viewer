#include "CoffaViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CoffaViewer w;
    w.showMaximized();
    return a.exec();
}
