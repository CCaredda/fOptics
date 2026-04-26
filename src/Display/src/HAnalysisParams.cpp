#include "HAnalysisParams.h"
#include "ui_HAnalysisParams.h"

HAnalysisParams::HAnalysisParams(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HAnalysisParams)
{
    ui->setupUi(this);

    //mode realse
    ui->_time_process->hide();

    //Send acquisition info
    connect(&_M_analyse,SIGNAL(newAcquisitionInfo(QString)),this,SIGNAL(newAcquisitionInfo(QString)));


    /************************************************************************
     ************************************************************************
     ********************Launch Programm**************************************
     ************************************************************************/

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

    //Optical device ID
    connect(this,SIGNAL(newOpticalConfigIDx(int,int)),&_M_analyse,SLOT(onNewOpticalConfigIDx(int,int)));


    /************************************************************************
     ************************************************************************
     ********************RESULTS*********************************************
     ************************************************************************/

    //Contrast image
    connect(&_M_analyse,SIGNAL(newContrastImage(QVector<Mat>)),this,SIGNAL(newContrastImage(QVector<Mat>)));



    //request results saving
    connect(this,SIGNAL(requestSaveResults(_data_saving_info)),&_M_analyse,SLOT(onrequestSaveResults(_data_saving_info)));

    /************************************************************************
     ************************************************************************
     ********************ANALYSIS********************************************
     ************************************************************************/

    //Chromophore ID
    connect(this,SIGNAL(newChromophoreID(int)),&_M_analyse,SLOT(onNewChromophoreID(int)));


    //init nb frames
    _M_nb_frames=0;

    //init FS
    _M_FS=0;

    //progress bar
    connect(&_M_analyse,SIGNAL(newProgressStatut(QString,int)),this,SIGNAL(newProgressValue(QString,int)));


    //Elapsed times
    connect(&_M_analyse,SIGNAL(Elapsed_ProcessingTime(int)),this,SLOT(SetElapsed_ProcessTime(int)));

   //Single point plot
   connect(&_M_analyse,SIGNAL(newContrastplot(QVector<QVector<float> >)),this,SIGNAL(newContrastplot(QVector<QVector<float> >)));

   //error message
   connect(&_M_analyse,SIGNAL(Error(QString)),this,SLOT(onNewErrorMessage(QString)));

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


    //Change z threshold value (displayed in interface when statistical significance changed)
    connect(&_M_analyse,SIGNAL(newZThresh(double)),this,SIGNAL(newZThreshValue(double)));

    //Stat zone setting
    connect(ui->_stats,SIGNAL(newSettingStatisticZone(double)),this,SIGNAL(newSettingStatisticZone(double)));
    connect(ui->_stats,SIGNAL(newSettingStatisticZone(double)),&_M_analyse,SLOT(onnewFHWM(double)));

    //New resels number
    connect(this,SIGNAL(newReselsNumber(int)),&_M_analyse,SLOT(onNewReselsNumber(int)));


    //stats type
    connect(ui->_stats,SIGNAL(newStatType(int)),&_M_analyse,SLOT(onnewStatType(int)));
    connect(ui->_stats,SIGNAL(newStatType(int)),this,SIGNAL(newStatType(int)));


}

HAnalysisParams::~HAnalysisParams()
{
    delete ui;
}


//Stop all acquisition threads
void HAnalysisParams::stop_threads()
{
    _M_analyse.stop_threads();
}

//Wait threads to be finished
bool HAnalysisParams::wait_threads()
{
    return _M_analyse.wait_threads();
}

/** set result directory */
void HAnalysisParams::onNewResultDirectory(QString v)
{
    _M_analyse.onNewResultDirectory(v);
}




//Set elapsed process time
void HAnalysisParams::SetElapsed_ProcessTime(int v)
{
    QString txt = "Processing time : "+QString::number(v)+ "ms";
    qDebug()<<txt;
}


//New process image
void HAnalysisParams::onNewProcessImage(_Processed_img img)
{
    _M_analyse.addDatas(img);
}


//Request analysis zone (v:0 drawing, v:4 Rect)
void HAnalysisParams::onAnalysisZoneRequested()
{
    if(_M_analyse.isLearningDone())
    {
        if(_showInfosMessage("Learning","Reference has already been learned, would you like to relearn ?",true)==0)
        {
            //init analysis class
            _M_analyse.Initialize();

            //Clear image and request ROI definition
            emit ClearImgRequested();
            emit AnalysisZoneRequested();
        }
    }
    else
    {
        //init analysis class
        _M_analyse.Initialize();

        //Clear image and request ROI definition
        emit ClearImgRequested();
        emit AnalysisZoneRequested();
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
