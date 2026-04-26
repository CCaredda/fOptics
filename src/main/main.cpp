#include "HMain.h"
#include <QtGlobal>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include "loadinfos.h"
#include <cstdlib>


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


    QString filename_css = getShareDirPath("Display")+"/style/display.css";
    qDebug()<<"Filename css: "<<filename_css;
    QFile file_css(filename_css);
    if (file_css.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file_css);
        a.setStyleSheet(stream.readAll());
    }


    HMain w;


   w.show();
    // w.showMaximized();

   int result = a.exec();


   std::quick_exit(result);
}
