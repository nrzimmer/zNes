#include "mainwindow.h"
#include <QApplication>

extern "C" {void reset6502z(void);}

int main(int argc, char *argv[])
{
//    reset6502z();
//    return 0;
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
