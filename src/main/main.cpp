#include "HMain.h"
#include <QtGlobal>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include "loadinfos.h"


int main(int argc, char *argv[])
{
    qDebug() << "main() started";

    #ifdef Q_OS_WIN
        qputenv("QT_MEDIA_BACKEND", "ffmpeg");
        qputenv("OMP_STACKSIZE", "256M"); // set once, applies to ALL omp parallel
    #endif

    QApplication a(argc, argv);
    QString file_icon = getShareDirPath("share")+"/files/icon.png";
    a.setWindowIcon(QIcon(file_icon));
    HMain w;


   w.show();
    // w.showMaximized();
    return a.exec();
}
