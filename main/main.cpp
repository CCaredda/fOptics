#include "HMain.h"
#include <QtGlobal>
#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    HMain w;


//    w.show();
    w.showMaximized();
    return a.exec();
}
