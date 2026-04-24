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

}

HMain::~HMain()
{
    delete ui;
}
