#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    //qWarning("Hello World");
    //QXBee xbee0("/dev/ttyUSB0");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
