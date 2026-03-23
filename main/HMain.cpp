#include "HMain.h"
#include "ui_HMain.h"
#include <QFile>
#include <QMessageBox>


HMain::HMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HMain)
{
    ui->setupUi(this);
    this->showMaximized();
//    this->showFullScreen();

    qRegisterMetaType<_Processed_img_HS>("_Processed_img_HS");
    qRegisterMetaType<_Processed_img>("_Processed_img");
    qRegisterMetaType<Mat>("Mat");
    qRegisterMetaType<QVector<Mat>>("QVector<Mat>");
    qRegisterMetaType<QVector<QVector<Mat> >>("QVector<QVector<Mat> >");
    qRegisterMetaType<QVector<float>>("QVector<float>");
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<QVector<double>>("QVector<double>");
    qRegisterMetaType<QVector<bool>>("QVector<bool>");
    qRegisterMetaType<QVector<QVector<float> >>("QVector<QVector<float> >");
    qRegisterMetaType<QVector<Vec3b>>("QVector<Vec3b>");
    qRegisterMetaType<QVector<QVector<int> >>("QVector<QVector<int> >");
    qRegisterMetaType<_data_saving_info>("_data_saving_info");




    //Check if Python venv exists
    QString path;
    #ifdef Q_OS_WIN
        path = qEnvironmentVariable("USERPROFILE")+"\\.venv\\Optics_venv\\Scripts\\activate";
    #elif defined(Q_OS_UNIX)
        path = qEnvironmentVariable("HOME") +"/.venv/Optics_venv/bin/activate";
    #endif

    qDebug()<<"Path check venv: "<<path;

    if (!QFile::exists(path))
    {
        QMessageBox::critical(nullptr, "Error - The application will not work", "Please install python venv Optics_venv");
        QCoreApplication::quit();
    }


}

HMain::~HMain()
{
    delete ui;
}
