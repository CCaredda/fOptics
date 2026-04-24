#include "HMainDisplay.h"
#include "ui_HMainDisplay.h"


HMainDisplay::HMainDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HMainDisplay)
{
    ui->setupUi(this);






    /*****************************************************************************************************
     *****************************************************************************************************
     *************************************TAB HANDLING****************************************************
     *****************************************************************************************************
     ****************************************************************************************************/

    //Display help
    connect(ui->_help,SIGNAL(pressed()),this,SLOT(onhelp_required()));


    //Main tab
    ui->_main_tab->setCurrentIndex(0);
    ui->_main_tab->setTabText(0,"Processing");
    ui->_main_tab->setTabText(1,"Plots");


    //Parameters tab
    ui->_parameters_tab->setCurrentIndex(0);
    ui->_parameters_tab->setTabText(0,"Data acquisition");
    ui->_parameters_tab->setTabText(1,"Data analysis");
    ui->_parameters_tab->setTabText(2,"Data saving");
    ui->_parameters_tab->setTabVisible(3, false);


    /*****************************************************************************************************
     *****************************************************************************************************
     *******************ACQUISITION*******************************E***************************************
     *****************************************************************************************************
     ****************************************************************************************************/

    //Optical device idx
    connect(ui->_params_acquisition,SIGNAL(newOpticalConfigIDx(int,int)),ui->_params_analysis,SIGNAL(newOpticalConfigIDx(int,int)));




    //ROI selection
    connect(ui->_displayResult,SIGNAL(newRectROI(QPoint,QPoint)),ui->_params_acquisition,SLOT(onNewRectROI(QPoint,QPoint)));
    connect(ui->_params_acquisition,SIGNAL(newRectROI(QPoint,QPoint)),ui->_displayResult,SLOT(onNewRectROI(QPoint,QPoint)));
    connect(ui->_params_acquisition,SIGNAL(onRectROIRequested()),ui->_displayResult,SIGNAL(RectROIRequested()));


    //New Display img
    connect(ui->_params_acquisition,SIGNAL(newDisplayImage(Mat)),ui->_displayResult,SLOT(onNewImage(Mat)));
    connect(ui->_params_acquisition,SIGNAL(newInitialImg(Mat)),ui->_displayResult,SLOT(onNewInitialImg(Mat)));
    connect(ui->_displayResult,SIGNAL(newInitialImg(Mat)),ui->_params_analysis,SLOT(onnewInitialImg(Mat)));
    connect(ui->_params_acquisition,SIGNAL(newInitialImg(Mat)),ui->_params_analysis,SLOT(onnewInitialImg(Mat)));


    //Clear img requested
    connect(ui->_params_acquisition,SIGNAL(ClearImgRequested()),ui->_displayResult,SIGNAL(ClearImg()));

    //New frame rate value
    onNewFrameRate(30);
    connect(ui->_params_acquisition,SIGNAL(newFrameRate(double)),this,SLOT(onNewFrameRate(double)));

    //process times
    connect(ui->_params_acquisition,SIGNAL(newProcessTimes(QVector<int>,double)),ui->_params_analysis,SLOT(onNewProcessTimes(QVector<int>,double)));
    connect(ui->_params_analysis,SIGNAL(newActivationtimes(QVector<double>)),ui->_plot,SLOT(onNewActivationTimes(QVector<double>)));

    //First and last frames for analysis
    connect(ui->_params_acquisition,SIGNAL(newFirstLastAnalysisFrames(int,int)),ui->_params_analysis,SLOT(onNewFirstLastAnalysisFrames(int,int)));

    //New Bold signal
    connect(ui->_params_analysis,SIGNAL(newBoldSignal(QVector<float>)),ui->_plot,SLOT(onnewBoldSignal(QVector<float>)));


    //filtering
    connect(ui->_params_acquisition,SIGNAL(enableFiltering(bool)),ui->_params_analysis,SIGNAL(enableFiltering(bool)));
    connect(ui->_params_acquisition,SIGNAL(enableLowPassFiltering(bool)),ui->_params_analysis,SIGNAL(enableLowPassFiltering(bool)));
    connect(ui->_params_acquisition,SIGNAL(enableDataCorrection(bool)),ui->_params_analysis,SIGNAL(enableDataCorrection(bool)));


    /*****************************************************************************************************
     *****************************************************************************************************
     ***************************************DATA PROCESSING***********************************************
     *****************************************************************************************************
     ****************************************************************************************************/

    //Analysis choice
    connect(ui->_params_acquisition,SIGNAL(AnalysisChoice(int)),ui->_params_analysis,SLOT(AnalysisChoice(int)));


    //Clear img requested
    connect(ui->_params_analysis,SIGNAL(ClearImgRequested()),ui->_displayResult,SIGNAL(ClearImg()));

    //Analysis zone
    //Request analysis zone drawing
    connect(ui->_params_analysis,SIGNAL(AnalysisZoneRequested()),ui->_displayResult,SIGNAL(onAnalysisZoneRequested()));
    //Send drawn analysis zone
    connect(ui->_displayResult,SIGNAL(newAnalysisZone(QVector<QPoint>,cv::Size)),this,SLOT(onNewAnalysisZone(QVector<QPoint>,cv::Size)));

    //request ROI drawing
    connect(ui->_params_acquisition,SIGNAL(requestAnalysisZoneDrawing()),ui->_params_analysis,SLOT(onAnalysisZoneRequested()));

    //Save img results
    connect(ui->_save_widget,SIGNAL(requestSaveResults(_data_saving_info)),ui->_params_analysis,SIGNAL(requestSaveResults(_data_saving_info)));

    //when Data Acquisition Is Ready
    connect(ui->_params_analysis,SIGNAL(DataAcquisitionIsReady()),ui->_params_acquisition,SLOT(onDataAcquisitionIsReady()));


    //Get acquired data registered or not
    connect(ui->_params_acquisition,SIGNAL(newProcessImage(_Processed_img)),ui->_params_analysis,SLOT(onNewProcessImage(_Processed_img)));

    //Contrast image
    connect(ui->_params_analysis,SIGNAL(newContrastImage(QVector<Mat>)),ui->_displayResult,SLOT(onnewContrastImg(QVector<Mat>)));




    /*****************************************************************************************************
     *****************************************************************************************************
     ***************************************PROGRESS BAR**************************************************
     *****************************************************************************************************
     ****************************************************************************************************/
    //Analysis
    ui->_label_analysis->setText("Analysis");
    ui->_progressBarAnalysis->setValue(0);

    //Acquire data
    ui->_progressBarDataAcquisition->setValue(0);

    //Acquisition process
    ui->_progressBarAcquisitionProcess->setValue(0);
    ui->_label_acquisition_process->setText("Registration");


    connect(ui->_params_analysis,SIGNAL(newProgressValue(QString,int)),this,SLOT(onNewAnalysisProgress(QString,int)));

    connect(ui->_params_acquisition,SIGNAL(newAcquisitionProcess(QString,int)),this,SLOT(onNewAcquisitionProcessProgress(QString,int)));
    connect(ui->_params_acquisition,SIGNAL(newProgressBarDataAcquisition(int)),ui->_progressBarDataAcquisition,SLOT(setValue(int)));

    /*****************************************************************************************************
     *****************************************************************************************************
     ***************************************DISPLAY PARAMS************************************************
     *****************************************************************************************************
     ****************************************************************************************************/

    //Chromophore ID
    connect(ui->_displayResult,SIGNAL(newChromophoreID(int)),ui->_params_analysis,SIGNAL(newChromophoreID(int)));

    //New max display values
    connect(ui->_displayResult,SIGNAL(newDisplayValue(double)),ui->_params_analysis,SLOT(onnewDisplayValue(double)));
    //request launch process
    connect(ui->_displayResult,SIGNAL(LaunchProcess()),ui->_params_analysis,SLOT(onLaunchProcess()));

    //emit point position
    connect(ui->_displayResult,SIGNAL(PointSelected(Point)),ui->_params_analysis,SLOT(onPointSelected(Point)));


    /************************************************************************
     ************************************************************************
     ********************PLOTS At selected Point*****************************
     ************************************************************************/

    //send point pos to analysis
    connect(ui->_displayResult,SIGNAL(PointSelected(Point)),ui->_params_analysis,SLOT(onPointSelected(Point)));


    QVector<QString> names;
    names .push_back("Delta[HbO2]");
    names .push_back("Delta[Hb]");
    names .push_back("Delta[HT]");
    ui->_plot->setPlotNames(names);
    ui->_plot->setYName("Concentration variation (µM)");
    ui->_plot->setXName("Time (s)");

    //X interval pour les graphes de conecntrations
    ui->_plot->setXInterval(1);
    ui->_plot->setXStart(0);

    //plot contrast at clicked point
    connect(ui->_params_analysis,SIGNAL(newContrastplot(QVector<QVector<float> >)),this,SLOT(onNewPlot(QVector<QVector<float> >)));

    //MEAN ROI
    connect(ui->_save_widget,SIGNAL(newMeanROIRadius(double)),ui->_displayResult,SIGNAL(newMeanROIRadius(double)));
    connect(ui->_save_widget,SIGNAL(newMeanROIRadius(double)),ui->_params_analysis,SIGNAL(newMeanROIRadius(double)));


    connect(ui->_save_widget,SIGNAL(requestMeanROIMeasure(bool)),ui->_displayResult,SIGNAL(requestMeanROI(bool)));
    connect(ui->_save_widget,SIGNAL(requestMeanROIMeasure(bool)),ui->_params_analysis,SIGNAL(requestMeanROIMeasure(bool)));


    /************************************************************************
     ************************************************************************
     ********************Define activated cortical areas*********************
     ************************************************************************/

    //Statistic zone
    connect(ui->_params_analysis,SIGNAL(newSettingStatisticZone(double)),ui->_displayResult,SIGNAL(newSettingStatisticZone(double)));
    connect(ui->_params_analysis,SIGNAL(newStatType(int)),ui->_displayResult,SLOT(onnewStatType(int)));

    //Statistic mask
    connect(ui->_displayResult,SIGNAL(newReselNumber(int)),ui->_params_analysis,SIGNAL(newReselsNumber(int)));

    //Statistic threhold (RFT thresh)
    connect(ui->_params_analysis,SIGNAL(newZThreshValue(double)),ui->_displayResult,SLOT(onNewZThresh(double)));


    /************************************************************************
     ************************************************************************
     ********************HMI MODE********************************************
     ************************************************************************/
    onNew_Guru_Mode(false);
    connect(ui->_HMI_mode,SIGNAL(clicked(bool)),this,SLOT(onNew_Guru_Mode(bool)));

    //Set result directory
    connect(ui->_params_acquisition,SIGNAL(newResultDirectory(QString)),this,SLOT(onNewResultDirectory(QString)));
}

HMainDisplay::~HMainDisplay()
{
    delete ui;
}


void HMainDisplay::onhelp_required()
{
    QString share_dir = getShareDirPath("Doc");
    if (share_dir=="")
        return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(share_dir+"/usage_doc.pdf"));
}

void HMainDisplay::onNewResultDirectory(QString v)
{

    ui->_displayResult->onNewResultDirectory(v);
    ui->_params_analysis->onNewResultDirectory(v);
}

//New HMI mode (user, guru)
void HMainDisplay::onNew_Guru_Mode(bool v)
{
    ui->_params_acquisition->onNew_Guru_Mode(v);
    ui->_displayResult->onNew_Guru_Mode(v);
}


//receive a new analysis zone
void HMainDisplay::onNewAnalysisZone(QVector<QPoint> roi,cv::Size s)
{
    qDebug()<<"[HMainDisplay] onNewAnalysisZone ";
    //Request image (nb channels) and acquisition (nb frames) infos
    int nb_channels = ui->_params_acquisition->getSpectralCompenentNumber();
    int nb_frames   = ui->_params_acquisition->getAcquisitionFrameNumber();

    if(nb_frames==0)
    {
        qDebug()<<"[HMainDisplay] nb frame = 0 ";
        return;
    }

    //Send ROI and nb channels and frames info to analysis
    ui->_params_analysis->onNewAnalysisZone(roi,s,nb_channels,nb_frames);

    //Send ROI vector to Acquisition class in order to process normalized cross correlation
    ui->_params_acquisition->newAnalysisZone(roi);
}



// Progress bar infos
//ANALYSIS
void HMainDisplay::onNewAnalysisProgress(QString msg,int v)
{
    ui->_label_analysis->setText(msg);
    ui->_progressBarAnalysis->setValue(v);
}


//Acquisition process
void HMainDisplay::onNewAcquisitionProcessProgress(QString msg,int v)
{
    ui->_label_acquisition_process->setText(msg);
    ui->_progressBarAcquisitionProcess->setValue(v);
}


//New frame rate
void HMainDisplay::onNewFrameRate(double v)
{
    ui->_params_analysis->onNewFrameRate(v);

    // X concentration plot
    ui->_plot->setXInterval((1/v));

    //Send to FFT Plot
    //X interval pour les graphes de conecntrations
    ui->_plot->setFrameRate(v);
}

//Display contrast at clicked point
void HMainDisplay::onNewPlot(QVector<QVector<float> > v)
{
    //Search min max values
    float min=0;
    float max=0;
    for(int i=0;i<v.size();i++)
    {
        for(int j=0;j<v[i].size();j++)
        {
            min = (v[i][j]<min) ? v[i][j] : min;
            max = (v[i][j]>max) ? v[i][j] : max;
        }
    }
    //set Y lim axis
    ui->_plot->setYInterval(min,max);

    //plot vec
    ui->_plot->setYValue(v);


    //set Plot stat
//    ui->_displayResult->setPlotStats(stat);
}




QString HMainDisplay::getResultDirectory()
{
    return ui->_params_acquisition->getResultDirectory();
}
