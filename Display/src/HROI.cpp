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
    connect(ui->_init_ROI,SIGNAL(pressed()),this,SIGNAL(InitROIRequested()));

    //Request analysis zone definition
    connect(ui->_draw_ROI,SIGNAL(pressed()),this,SLOT(requestAnalysisZoneDrawing()));


    //Maxiumum number of analyzed data
    ui->_widget_data_number->hide();
    ui->_maximum_number_data->setIntegerMode();
    ui->_maximum_number_data->setRange(20000,1000000);
    ui->_maximum_number_data->setValue(50000);
    connect(ui->_maximum_number_data,SIGNAL(valueEdited(double)),this,SIGNAL(maximumNumberDataChanged(double)));
}

HROI::~HROI()
{
    delete ui;
}


//request ROI zone drawing (Manual drawing or rectangular zone)
void HROI::requestAnalysisZoneDrawing()
{
    emit requestAnalysisZoneDrawing(0);
}







void HROI::onNew_Guru_Mode(bool v)
{
    if (v)
        ui->_widget_data_number->show();
    else
        ui->_widget_data_number->hide();

//    if(v)
//    {
//        ui->_ROI_extraction_method->show();
//    }
//    else
//    {
//        ui->_ROI_extraction_method->hide();
//    }
}

void HROI::enableButtons(bool v)
{
    ui->_pre_ROI->setEnabled(v);
    ui->_init_ROI->setEnabled(v);
    ui->_draw_ROI->setEnabled(v);
}


