#include "HSaveData.h"
#include "ui_HSaveData.h"

HSaveData::HSaveData(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HSaveData)
{
    ui->setupUi(this);

    /************************************************************************
     ************************************************************************
     ********************Save Results****************************************
     ************************************************************************/
    connect(ui->_save_results,SIGNAL(pressed()),this,SLOT(_requestSaveResults()));


    /************************************************************************
     ************************************************************************
     ********************Mean ROI ********************************************
     ************************************************************************/
    //Enable Mean ROI
    ui->_Mean_ROI_box->setChecked(false);
    onEnableMeanROI(false);
    connect(ui->_Mean_ROI_box,SIGNAL(clicked(bool)),this,SLOT(onEnableMeanROI(bool)));

    //ROI radius
    ui->_radius_Mean_ROI->setRange(1,100,"px");
    ui->_radius_Mean_ROI->setIntegerMode();
    ui->_radius_Mean_ROI->setValue(5);
    connect(ui->_radius_Mean_ROI,SIGNAL(valueEdited(double)),this,SIGNAL(newMeanROIRadius(double)));
}


HSaveData::~HSaveData()
{
    delete ui;
}


/** Request results saving **/
void HSaveData::_requestSaveResults()
{
    _data_saving_info info;
    info.save_camera_intensity      = ui->_intensity->isChecked();
    info.save_Delta_C               = ui->_Delta_C->isChecked();

    emit requestSaveResults(info);
}

//Enable mean roi box
void HSaveData::onEnableMeanROI(bool v)
{
    ui->_Mean_ROI->setEnabled(v);
    emit requestMeanROIMeasure(v);
}



