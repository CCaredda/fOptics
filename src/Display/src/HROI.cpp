#include "HROI.h"
#include "ui_HROI.h"

HROI::HROI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HROI)
{
    ui->setupUi(this);


    //New ROI selection
    connect(ui->_pre_ROI,SIGNAL(pressed()),this,SIGNAL(PreROIRequested()));

    //Init ROI
    connect(ui->_init_ROI,SIGNAL(pressed()),this,SLOT(onInitROIRequested()));

    //Request analysis zone definition
    connect(ui->_draw_ROI,SIGNAL(pressed()),this,SIGNAL(requestAnalysisZoneDrawing()));


    //Maxiumum number of analyzed data
    ui->_widget_data_number->hide();
    ui->_maximum_number_data->setIntegerMode();
    ui->_maximum_number_data->setRange(20000,1000000);
    ui->_maximum_number_data->setValue(50000);
    connect(ui->_maximum_number_data,SIGNAL(valueEdited(double)),this,SIGNAL(maximumNumberDataChanged(double)));

    ui->_draw_ROI->setEnabled(false);
    ui->_init_ROI->setEnabled(true);
    ui->_pre_ROI->setEnabled(true);

    enableButtons(false);
}

HROI::~HROI()
{
    delete ui;
}

void HROI::init_Interface()
{
    qDebug()<<"In HROI::init_Interface";

    this->setEnabled(true);
    ui->_draw_ROI->setEnabled(false);
    ui->_init_ROI->setEnabled(true);
    ui->_pre_ROI->setEnabled(true);
}



void HROI::onInitROIRequested()
{
    emit InitROIRequested();
    ui->_draw_ROI->setEnabled(false);
    ui->_init_ROI->setEnabled(true);
    ui->_pre_ROI->setEnabled(true);
}

void HROI::onROI_drawn()
{
    this->setEnabled(false);
}

void HROI::onRectROI_drawn()
{
    ui->_init_ROI->setEnabled(true);
    ui->_pre_ROI->setEnabled(false);
    ui->_draw_ROI->setEnabled(true);
}




void HROI::onNew_Guru_Mode(bool v)
{
    if (v)
        ui->_widget_data_number->show();
    else
        ui->_widget_data_number->hide();
}

void HROI::enableButtons(bool v)
{
    ui->_pre_ROI->setEnabled(v);
    ui->_init_ROI->setEnabled(v);
    ui->_draw_ROI->setEnabled(v);
}

