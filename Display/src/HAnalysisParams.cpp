#include "HAnalysisParams.h"
#include "ui_HAnalysisParams.h"

HAnalysisParams::HAnalysisParams(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HAnalysisParams)
{
    ui->setupUi(this);


    //requestion acquisition info
    connect(this,SIGNAL(requestAcquisitionInfo(bool)),&_M_analyse,SIGNAL(requestAcquisitionInfo(bool)));

    //Send acquisition info
    connect(&_M_analyse,SIGNAL(newAcquisitionInfo(QString)),this,SIGNAL(newAcquisitionInfo(QString)));


    /************************************************************************
     ************************************************************************
     ********************Launch Programm**************************************
     ************************************************************************/
    _M_motion_learned = false;
    _M_data_ready = false;

    //get signal that analysis class is ready to store datas
    //and communicate it to acquisitions class
    connect(&_M_analyse,SIGNAL(DataAcquisitionIsReady()),this,SIGNAL(DataAcquisitionIsReady()));
//    connect(&_M_analyse,SIGNAL(DataAcquisitionIsReady()),this,SLOT(onDataAcquisitionIsReady()));



    //Filtering
    connect(this,SIGNAL(enableFiltering(bool)),&_M_analyse,SLOT(onEnableFiltering(bool)));
    connect(this,SIGNAL(enableLowPassFiltering(bool)),&_M_analyse,SLOT(onEnableLowPassFiltering(bool)));
    connect(this,SIGNAL(enableDataCorrection(bool)),&_M_analyse,SLOT(onEnableDataCorrection(bool)));

    //Activation times
    connect(&_M_analyse,SIGNAL(newActivationtimes(QVector<double>)),this,SIGNAL(newActivationtimes(QVector<double>)));
    connect(ui->_time_process,SIGNAL(newActivationStepsToConsider(int)),&_M_analyse,SLOT(onNewActivationStepsToConsider(int)));

    //Bold signal
    connect(&_M_analyse,SIGNAL(newBoldSignal(QVector<float>)),this,SIGNAL(newBoldSignal(QVector<float>)));

    /************************************************************************
     ************************************************************************
     ********************SEGMENTATION****************************************
     ************************************************************************/

    //Segmentation layers
    connect(this,SIGNAL(newSegmentationLayers(Mat,Mat,Mat)),&_M_analyse,SLOT(onnewSegmentationLayers(Mat,Mat,Mat)));
    connect(this,SIGNAL(ConsiderBloodVesselDistance(bool)),&_M_analyse,SIGNAL(ConsiderBloodVesselDistance(bool)));



    //Hyperspectral segmentation
    connect(&_M_analyse,SIGNAL(newHyperspectralSegmentation(QVector<int>)),this,SIGNAL(newHyperspectralSegmentation(QVector<int>)));


    /************************************************************************
     ************************************************************************
     ********************RESULTS*********************************************
     ************************************************************************/

    //Contrast image
    connect(&_M_analyse,SIGNAL(newContrastImage(QVector<Mat>)),this,SIGNAL(newContrastImage(QVector<Mat>)));

    //Activation maps
    connect(&_M_analyse,SIGNAL(newActivationMap(QVector<bool>)),this,SIGNAL(newActivationMap(QVector<bool>)));
    connect(&_M_analyse,SIGNAL(newActivationMap(QVector<bool>,int)),this,SIGNAL(newActivationMap(QVector<bool>,int)));
    connect(&_M_analyse,SIGNAL(newActivationMap(Mat)),this,SIGNAL(newActivationMap(Mat)));

    //request results saving
    connect(this,SIGNAL(requestSaveResults(_data_saving_info)),&_M_analyse,SLOT(onrequestSaveResults(_data_saving_info)));
    connect(this,SIGNAL(requestNonFilteredDataSaving(bool)),&_M_analyse,SLOT(onrequestNonFilteredDataSaving(bool)));

    /************************************************************************
     ************************************************************************
     ********************ANALYSIS********************************************
     ************************************************************************/


    //Chromophore ID
    connect(this,SIGNAL(newChromophoreID(int)),&_M_analyse,SLOT(onNewChromophoreID(int)));

    //First Hyperspectral img
    connect(this,SIGNAL(newFirstHSImage(vector<Mat>)),&_M_analyse,SLOT(onnewFirstHSImage(vector<Mat>)));


    //init nb frames
    _M_nb_frames=0;

    //init FS
    _M_FS=0;

    //progress bar
    connect(&_M_analyse,SIGNAL(newProgressStatut(QString,int)),this,SIGNAL(newProgressValue(QString,int)));

    //Elapsed times
    connect(&_M_analyse,SIGNAL(Elapsed_ProcessingTime(int)),this,SLOT(SetElapsed_ProcessTime(int)));


//   //new image cluster
//   connect(&_M_analyse,SIGNAL(newImageCluster(Mat)),this,SIGNAL(newImageCluster(Mat)));

   //Single point plot
   connect(&_M_analyse,SIGNAL(newContrastplot(QVector<QVector<float> >)),this,SIGNAL(newContrastplot(QVector<QVector<float> >)));

   //error message
   connect(&_M_analyse,SIGNAL(Error(QString)),this,SLOT(onNewErrorMessage(QString)));

   //new correlation threshold
   _M_analyse.onNewCorrelationThreshold(0.2);
   connect(this,SIGNAL(newCorrelationThreshold(double)),&_M_analyse,SLOT(onNewCorrelationThreshold(double)));

   //New pixel position
   connect(&_M_analyse,SIGNAL(newPixelPos(QVector<Point>)),this,SIGNAL(newPixelPos(QVector<Point>)));


  /************************************************************************
   ************************************************************************
   ********************Mean ROI********************************************
   ************************************************************************/
   //Radius
   connect(this,SIGNAL(newMeanROIRadius(double)),&_M_analyse,SLOT(onnewMeanROIRadius(double)));
   //Enable measurement
   connect(this,SIGNAL(requestMeanROIMeasure(bool)),&_M_analyse,SLOT(onrequestMeanROIMeasure(bool)));





   /************************************************************************
    ************************************************************************
    ********************Define cortical area********************************
    ************************************************************************/
    ui->_stats->enableActivatedAreasDefinition(false);

    //Statistical significance
//    connect(ui->_stats,SIGNAL(newStatisticalLevel(int)),&_M_analyse,SLOT(onnewStatisticalLevel(int)));

    //Change z threshold value (displayed in interface when statistical significance changed)
    connect(&_M_analyse,SIGNAL(newZThresh(double)),this,SIGNAL(newZThreshValue(double)));

    //init stats zone
//    connect(ui->_stats,SIGNAL(requestStatsZoneInit()),&_M_analyse,SLOT(initStatsZone()));
//    connect(ui->_stats,SIGNAL(requestStatsZoneInit()),this,SIGNAL(requestStatsZoneInit()));

    //Stat zone setting
    connect(ui->_stats,SIGNAL(newSettingStatisticZone(double)),this,SIGNAL(newSettingStatisticZone(double)));
    connect(ui->_stats,SIGNAL(newSettingStatisticZone(double)),&_M_analyse,SLOT(onnewFHWM(double)));


    //Get filtered and non filtered data
    connect(ui->_stats,SIGNAL(RequireFilteredNonFilteredSignals(bool)),&_M_analyse,SLOT(onRequireFilteredNonFilteredSignals(bool)));

    //Get distance to the nearest blood vessel
    connect(ui->_stats,SIGNAL(RequireDistanceToBloodVessels(bool)),&_M_analyse,SLOT(onRequireDistanceToBloodVessels(bool)));

    //stats type
    connect(ui->_stats,SIGNAL(newStatType(int)),&_M_analyse,SLOT(onnewStatType(int)));
    connect(ui->_stats,SIGNAL(newStatType(int)),this,SIGNAL(newStatType(int)));


    /************************************************************************
     ************************************************************************
     *********************Resting state**************************************
     ************************************************************************/
    //enable resting state
    connect(ui->_stats,SIGNAL(enableRestingState(bool)),&_M_analyse,SLOT(onenableRestingState(bool)));
    connect(ui->_stats,SIGNAL(enableRestingState(bool)),this,SIGNAL(enableRestingState(bool)));

//    //process
//    connect(ui->_stats,SIGNAL(requestRestingStateProcessing()),this,SIGNAL(requestSeeds()));

    //init seeds
    connect(ui->_stats,SIGNAL(requestSeedInit()),this,SIGNAL(requestSeedInit()));

    //Seed radius
    connect(ui->_stats,SIGNAL(newSeedRadius(double)),this,SIGNAL(newSeedRadius(double)));

    //Grid seeds
    connect(ui->_stats,SIGNAL(newGridSeeds(bool)),this,SIGNAL(newGridSeeds(bool)));

    //Maps
    connect(&_M_analyse,SIGNAL(newRestingStateMaps(QVector<QVector<Mat> >)),this,SIGNAL(newRestingStateMaps(QVector<QVector<Mat> >)));
    connect(&_M_analyse,SIGNAL(newKmeans_RestingStateMaps(QVector<Mat>)),this,SIGNAL(newKmeans_RestingStateMaps(QVector<Mat>)));

    //Keep seed connexion
    connect(ui->_stats,SIGNAL(requestSeedConnexion(bool)),this,SIGNAL(requestSeedConnexion(bool)));

    //Seed extraction: Thee new seed become the resting state map in which the seed is included
    connect(ui->_stats,SIGNAL(requestSeedsExtraction()),this,SIGNAL(requestSeedsExtraction()));

    //New resting state method (0: seed based method, 1: ICA based method, 2: frequency power, 3: K-means clustering)
    connect(ui->_stats,SIGNAL(newRestingStateMethod(int)),this,SIGNAL(newRestingStateMethod(int)));
    connect(ui->_stats,SIGNAL(newRestingStateMethod(int)),this,SLOT(onnewRestingStateMethod(int)));

    //new cluster nb (K-means method)
    connect(ui->_stats,SIGNAL(newClustersRestingState(double)),&_M_analyse,SLOT(onnewClustersRestingState(double)));


    //new nb of independant sources (ICA)
    connect(ui->_stats,SIGNAL(newNbofIndependantSources_ICA(double)),this,SLOT(onnewNbofIndependantSources_ICA(double)));

    //new ICA source of interest
    connect(ui->_stats,SIGNAL(newICASourceofInterest(int)),this,SIGNAL(newICASourceofInterest(int)));

    //New resting state map sampling
    connect(ui->_stats,SIGNAL(NewRestingStateMapsSampling(double)),&_M_analyse,SLOT(onNewRestingStateMapsSampling(double)));


   /************************************************************************
   ************************************************************************
   ********************Mode Hyperspectral**********************************
   ************************************************************************/


  //Hyperspectral spectral range
  connect(this,SIGNAL(newSpectralRange(int,int)),&_M_analyse,SLOT(onnewSpectralRange(int,int)));

  //Spectral correction
  connect(this,SIGNAL(requireSpectralCorrection(bool)),&_M_analyse,SLOT(onRequireSpectralCorrection(bool)));

  //Optical device ID
  connect(this,SIGNAL(newOpticalConfigIDx(int,int)),&_M_analyse,SLOT(onNewOpticalConfigIDx(int,int)));



  /************************************************************************
  ************************************************************************
  ********************Create video****************************************
  ************************************************************************/
    //launch video writting
    connect(this,SIGNAL(VideoCreationRequired()),&_M_analyse,SLOT(onCreateNewVideo()));
    connect(this,SIGNAL(newVideo_nb_Frames(double)),&_M_analyse,SLOT(onnewVideo_nb_Frames(double)));
    connect(this,SIGNAL(newVideoFramerate(double)),&_M_analyse,SLOT(onnewVideoFramerate(double)));

}

HAnalysisParams::~HAnalysisParams()
{
    delete ui;
}



/** set result directory */
void HAnalysisParams::onNewResultDirectory(QString v)
{
    _M_analyse.onNewResultDirectory(v);
}




//Set HMI mode (user, guru)
void HAnalysisParams::onNew_Guru_Mode(bool v)
{
    ui->_stats->onNew_Guru_Mode(v);
}



//new activated cortical area definition
void HAnalysisParams::onnewCorticalAreaDefinition(Rect ref,QVector<Rect> cort,QVector<Mat> cort_residu)
{
    _M_analyse.onnewCorticalAreaDefinition(ref,cort,cort_residu);
}

void HAnalysisParams::onnewCorticalAreaDefinition(QVector<Rect> cort,QVector<Mat> cort_residu)
{
    _M_analyse.onnewCorticalAreaDefinition(cort,cort_residu);
}


//Set elapsed process time
void HAnalysisParams::SetElapsed_ProcessTime(int v)
{
    QString txt = "Processing time : "+QString::number(v)+ "ms";
    qDebug()<<"[HAnalysisParams]"<<txt;
}


//New process image
void HAnalysisParams::onNewProcessImage(_Processed_img img)
{
    _M_analyse.addDatas(img);
}

void HAnalysisParams::onNewProcessImage_HS(_Processed_img_HS img)
{
    _M_analyse.addDatas(img);
}

//Request analysis zone (v:0 drawing, v:4 Rect)
void HAnalysisParams::onAnalysisZoneRequested(int v)
{
    if(_M_analyse.isLearningDone())
    {
        if(_showInfosMessage("Learning","Reference has already been learned, would you like to relearn ?",true)==0)
        {
            //init analysis class
            _M_analyse.Initialize();

            //Clear image and request ROI definition
            emit ClearImgRequested();
            emit AnalysisZoneRequested(v);
        }
    }
    else
    {
        //init analysis class
        _M_analyse.Initialize();

        //Clear image and request ROI definition
        emit ClearImgRequested();
        emit AnalysisZoneRequested(v);
    }
}

//New analysis zone received
void HAnalysisParams::onNewAnalysisZone(QVector<QPoint> roi,Size s,int nb_channels,int nb_frames)
{
    ui->_stats->enableActivatedAreasDefinition(true);
    _M_analyse.setAnalysisZone(roi,s,nb_channels,nb_frames);
    _M_nb_frames    = nb_frames;

}

/*
//Motion learning is finished,
void HAnalysisParams::onMotionLearningisFinished()
{
    _M_motion_learned = true;

    qDebug()<<"[HAnalysisParams::onMotionLearningisFinished]";
    if(_M_motion_learned && _M_data_ready)
    {
        qDebug()<<"emit DataAcquisitionIsReady signal";
        emit DataAcquisitionIsReady();
    }
}

//Data is ready
void HAnalysisParams::onDataAcquisitionIsReady()
{
    _M_data_ready = true;
    qDebug()<<"[HAnalysisParams::onDataAcquisitionIsReady]";

    if(_M_motion_learned && _M_data_ready)
    {
        qDebug()<<"emit DataAcquisitionIsReady signal";
        emit DataAcquisitionIsReady();
    }
}
*/

//Display Message
int HAnalysisParams::_showInfosMessage(QString text, QString infoText,bool YesNoQuestion)
{
    QString infoStyle(
                "QLabel { color: #b1b1b1;} \n"
                "QMessageBox {background-color: #464646; font-size:26px; font-weight:bold;} \n"
                "QPushButton {color: #b1b1b1; padding-left: 10px; padding-right: 10px; padding-top: 2px; padding-bottom: 2px ;  border: 2px solid #46d4d4; border-radius: 6; font-size:22px; \n"
                "background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #565656, stop: 0.1 #525252, stop: 0.5 #4e4e4e, stop: 0.9 #4a4a4a, stop: 1 #464646);} \n"
                "QPushButton:pressed {background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #2d2d2d, stop: 0.1 #2b2b2b, stop: 0.5 #292929, stop: 0.9 #282828, stop: 1 #252525);} ");

    QMessageBox messageInfo;
    messageInfo.setIcon(QMessageBox::Information);
    messageInfo.setText(text);
    messageInfo.setInformativeText(infoText);
    messageInfo.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);

    if(YesNoQuestion)
    {
        messageInfo.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        messageInfo.setDefaultButton(QMessageBox::Yes);
        messageInfo.setButtonText(QMessageBox::No, tr("No"));
        messageInfo.setButtonText(QMessageBox::Yes, tr("Yes"));
        messageInfo.setStyleSheet(infoStyle);
    }
    else
    {
        messageInfo.setStandardButtons(QMessageBox::Yes);
        messageInfo.setDefaultButton(QMessageBox::Yes);
        messageInfo.setButtonText(QMessageBox::Yes, tr("Yes"));
        messageInfo.setStyleSheet(infoStyle);
    }
    int ret = messageInfo.exec();
    if (ret == QMessageBox::Yes)
        return 0;
    else
        return -1;
}


//Launch process
void HAnalysisParams::onLaunchProcess()
{
    _M_analyse.LaunchProcess();
}



//Set First and last frames for analysis
void HAnalysisParams::onNewFirstLastAnalysisFrames(int start,int end)
{
    _M_analyse.setFirstLastFrameForAnalysis(start, end);
}


//New process times (define by camera acquistion)
void HAnalysisParams::onNewProcessTimes(QVector<int> time,double Fs)
{
    qDebug()<<"[HAnalysisParams] onNewProcessTimes. Fs: "<<Fs;
    //Set frame rate
    _M_analyse.setFrameRate(Fs);

    //Set process times
    _M_analyse.setProcessTimes(time);
    ui->_time_process->setNbActivityPeriod(floor(time.size()/2));
}

//Error message
void HAnalysisParams::onNewErrorMessage(QString v)
{
    _showInfosMessage("Error",v,false);
}


//new frame rate value
void HAnalysisParams::onNewFrameRate(double v)
{
    qDebug()<<"[HAnalysisParams] onNewFrameRate "<<v;
    _M_FS = (int)v;
    _M_analyse.setFrameRate(v);
//    onProcessTimeChanged();
}

////Cluster choice changed
//void HAnalysisParams::clusterChoiceChanged(QVector<int> v)
//{
//    _M_analyse.newClusterChoice(v);
//    _M_analyse.LaunchProcess();
//}

//On point selected
void HAnalysisParams::onPointSelected(Point p)
{
    _M_analyse.setStudiedPoint(p);
}


void HAnalysisParams::AnalysisChoice(int v)
{
    //-1: no analysis
    //0 : task-based
    //1: resting state
    //2: Impulsion
    ui->_stats->AnalysisChoice(v);
    if(v != 0)
        ui->_time_process->setEnabled(false);

}
