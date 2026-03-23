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

    //Enable or disable option for saving non filtered signals
    connect(ui->_non_filtered_data,SIGNAL(clicked(bool)),this,SIGNAL(requestNonFilteredDataSaving(bool)));

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

    /************************************************************************
    ************************************************************************
    ********************Create video****************************************
    ************************************************************************/
    //launch video writting
    connect(ui->_video,SIGNAL(VideoCreationRequired()),this,SIGNAL(VideoCreationRequired()));
    connect(ui->_video,SIGNAL(newVideo_nb_Frames(double)),this,SIGNAL(newVideo_nb_Frames(double)));
    connect(ui->_video,SIGNAL(newVideoFramerate(double)),this,SIGNAL(newVideoFramerate(double)));
}


HSaveData::~HSaveData()
{
    delete ui;
}

/** on Analysis choice (1: task-based) (2: resting-state), (3: impulsion)*/
void HSaveData::AnalysisChoice(int v)
{
    if(v != 0) //If not task-based analysis
    {
        ui->_Delta_C_rest_activity->setEnabled(false);
        ui->_SPM->setEnabled(false);
    }
}


// Acquisition launch: disable option for saving non filtered signal.
//This has to be done before acquiring data
//If option has been checked before acquiring, it is possible to check and uncheck the box since data have been stored in RAM
void HSaveData::onDataAcquisitionIsReady()
{
    if(!ui->_non_filtered_data->isChecked())
        ui->_non_filtered_data->setEnabled(false);
}

/** Request results saving **/
void HSaveData::_requestSaveResults()
{
    _data_saving_info info;
    info.save_camera_intensity      = ui->_intensity->isChecked();
    info.save_Delta_A               = ui->_Delta_A->isChecked();
    info.save_Delta_C               = ui->_Delta_C->isChecked();
    info.save_Delta_C_rest_activity = ui->_Delta_C_rest_activity->isChecked();
    info.save_SPM                   = ui->_SPM->isChecked();
    info.save_non_filtered_data     = ui->_non_filtered_data->isChecked();

    emit requestSaveResults(info);
}

//Enable mean roi box
void HSaveData::onEnableMeanROI(bool v)
{
    ui->_Mean_ROI->setEnabled(v);
    emit requestMeanROIMeasure(v);
}



