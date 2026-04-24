#include "HTimeProcess.h"
#include "ui_HTimeProcess.h"

HTimeProcess::HTimeProcess(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HTimeProcess)
{
    ui->setupUi(this);

    //Activity period
    connect(ui->_paradigm,SIGNAL(currentIndexChanged(int)),this,SIGNAL(newActivationStepsToConsider(int)));
    setNbActivityPeriod(1);

    ui->_main_widget->hide();
}

HTimeProcess::~HTimeProcess()
{
    delete ui;
}


void HTimeProcess::setNbActivityPeriod(int v)
{
    ui->_main_widget->show();
    qDebug()<<"HTimeProcess::setNbActivityPeriod "<<v;
    if(v==1)
        ui->_paradigm->hide();
    else
        ui->_paradigm->show();

    ui->_paradigm->clear();
    for(int i=0;i<v;i++)
        ui->_paradigm->addItem("1 to "+QString::number(i+1));

    ui->_paradigm->setCurrentIndex(ui->_paradigm->count()-1);
    emit newActivationStepsToConsider(ui->_paradigm->count()-1);
}


