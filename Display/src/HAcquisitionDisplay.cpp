#include "HAcquisitionDisplay.h"
#include "ui_HAcquisitionDisplay.h"

HAcquisitionDisplay::HAcquisitionDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HAcquisitionDisplay)
{
    ui->setupUi(this);

    //Init result directory
    _M_result_directory =  "/home/results/";

    /**********************************/
    /*******General controls***********/
    /**********************************/

    //Guru mode
    onNew_Guru_Mode(false);


    //Acquisition mode
    //checked : mode caméra
    //unchecked : load image from hard drive
//    ui->_mode_camera_Img->setChecked(false);
    onNewAcquisitionMode(false);
//    connect(ui->_mode_camera_Img,SIGNAL(clicked(bool)),this,SLOT(onNewAcquisitionMode(bool)));

    //Mode RGB
    connect(&_M_load_datas_hard_drive,SIGNAL(newRGBCameraMode(bool)),this,SLOT(onnewRGBMode(bool)));

    /**********************************/
    /*******Post acquisition***********/
    /**********************************/

    //Image acquired
    connect(&_M_load_datas_hard_drive,SIGNAL(newImageAcquired(_Processed_img)),&_M_post_acquis,SLOT(newImage(_Processed_img)));
    // connect(&_M_load_datas_hard_drive,SIGNAL(newImageAcquired(Mat,int)),&_M_post_acquis,SLOT(newImage(Mat,int)));


    //Send image to registration class
    connect(&_M_post_acquis,SIGNAL(newPreProcessedImg(_Processed_img)),&_M_reg,SLOT(requestParallelThreadProcess(_Processed_img)));
    connect(&_M_post_acquis,SIGNAL(newPreProcessedImg(_Processed_img_HS)),&_M_reg,SLOT(requestParallelThreadProcess(_Processed_img_HS)));

    //Request first image pre processing
    connect(&_M_load_datas_hard_drive,SIGNAL(requestPreProcessing(Mat)),&_M_post_acquis,SLOT(requestPreProcessing(Mat)));

    //New display Images
    connect(&_M_post_acquis,SIGNAL(newDisplayImage(Mat)),this,SIGNAL(newDisplayImage(Mat)));
    connect(&_M_post_acquis,SIGNAL(newInitialImg(Mat)),this,SIGNAL(newInitialImg(Mat)));




    /********************************************/
    /*******Contour and ROI detection************/
    /********************************************/


    //Request analysis zone definition
    connect(ui->_ROI,SIGNAL(requestAnalysisZoneDrawing(int)),this,SIGNAL(requestAnalysisZoneDrawing(int)));

    //_ROI_extraction_method
    connect(ui->_ROI,SIGNAL(newROIExtractionMethod(int)),this,SLOT(onNewROIExtractionMethod()));

    /*********************************/
    /*******Registration**************/
    /*********************************/

    //Require First img
    _M_require_Learning_First_img=false;


    //registration progress
    connect(&_M_reg,SIGNAL(newRegistrationProgress(QString,int)),this,SIGNAL(newAcquisitionProcess(QString,int)));


    //Send registered image to analysis
    connect(&_M_reg,SIGNAL(newRegisteredImage(_Processed_img)),this,SIGNAL(newProcessImage(_Processed_img)));
    connect(&_M_reg,SIGNAL(newRegisteredImage(_Processed_img_HS)),this,SIGNAL(newProcessImage_HS(_Processed_img_HS)));

    /*********************************/
    /*******Pre ROI*******************/
    /*********************************/

    //Request Pre-ROI
    connect(ui->_ROI,SIGNAL(PreROIRequested()),this,SIGNAL(onRectROIRequested()));

    //Init Pre-ROI
    connect(ui->_ROI,SIGNAL(InitROIRequested()),&_M_post_acquis,SLOT(InitROI()));

    //Maximum number of data in pre ROI rect
    connect(ui->_ROI,SIGNAL(maximumNumberDataChanged(double)),&_M_post_acquis,SLOT(on_New_Maximum_number_Analyzed_pixels(double)));

    /*********************************/
    /*******Load Datas from Files*****/
    /*********************************/

    //Message from AloadDatas class
    connect(&_M_load_datas_hard_drive,SIGNAL(newMessage(QString)),ui->_param_LoadImg,SLOT(onNewMessage(QString)));

    //Set directory path
    connect(ui->_param_LoadImg,SIGNAL(newVideoPath(QString)),&_M_load_datas_hard_drive,SLOT(setVideoPath(QString)));

    //Set result directory
    connect(ui->_param_LoadImg,SIGNAL(newResultDirectory(QString)),this,SLOT(onNewResultDirectory(QString)));

    //NewImgLoaded
    connect(&_M_load_datas_hard_drive,SIGNAL(newImgSend(int,int)),this,SLOT(updateDataAcquisitionProgressBar(int,int)));


    /*******************************************************/
    /*******Acquisition infos*******************************/
    /*******************************************************/

    //Frame rate
    connect(&_M_load_datas_hard_drive,SIGNAL(newFrameRate(double)),this,SIGNAL(newFrameRate(double)));

    //Process times
    connect(&_M_load_datas_hard_drive,SIGNAL(newProcessid(QVector<int>,double)),this,SIGNAL(newProcessTimes(QVector<int>,double)));

    //First and last analysis frames
    connect(&_M_load_datas_hard_drive,SIGNAL(newFirstLastAnalysisFrames(int,int)),this,SIGNAL(newFirstLastAnalysisFrames(int,int)));

    //enable filtering
    connect(ui->_enable_filtering,SIGNAL(clicked(bool)),this,SIGNAL(enableFiltering(bool)));
    connect(ui->_enable_low_pass_filtering,SIGNAL(clicked(bool)),this,SIGNAL(enableLowPassFiltering(bool)));
    connect(ui->_enable_data_correction,SIGNAL(clicked(bool)),this,SIGNAL(enableDataCorrection(bool)));


    //enable motion compensation
    connect(ui->_enable_Motion_Compensation,SIGNAL(clicked(bool)),&_M_reg,SLOT(enableMotionCompensation(bool)));



    //Request thread info
    // connect(ui->_acquisition_info,SIGNAL(clicked(bool)),this,SIGNAL(requestAcquisitionInfo(bool)));




    /*******************************************************/
    /*******Acquisition configuration***********************/
    /*******************************************************/


    //analysis type
    connect(&_M_load_datas_hard_drive,SIGNAL(requestAnalysisChoose(QVector<int>)),this,SLOT(requestAnalysisChoose(QVector<int>)));
    connect(&_M_choose_analysis,SIGNAL(enableAnalysis(int)),this,SLOT(enableAnalysis(int)));

}

HAcquisitionDisplay::~HAcquisitionDisplay()
{
    delete ui;
}

/** New result directory */
void HAcquisitionDisplay::onNewResultDirectory(QString v)
{
    _M_result_directory = v;
    _M_reg.onNewResultDirectory(v);
    _M_post_acquis.onNewResultDirectory(v);

    emit newResultDirectory(v);
}


/** New acquisition info */
void HAcquisitionDisplay::onNewAcquisitionInfo(QString msg)
{
    ui->_info->setText(msg);
}







//on new RGB mode
void HAcquisitionDisplay::onnewRGBMode(bool)
{
    //Check camera type
    onCheckCamera_ID();
}


/** Check camera ID in metadata (RGB, HS, etc)
 Remove bool and check metadata*/
void HAcquisitionDisplay::onCheckCamera_ID()
{
    qDebug()<<"HAcquisitionDisplay::onCheckCamera_ID";

    //light source type
    // emit newLightSourceID(Light_source_Halogen);
    QString light_source_type = _M_load_datas_hard_drive.get_Light_source_type();
    int code_light_source = decode_Light_source_type(light_source_type);
    qDebug()<<"[HAcquisitionDisplay::onCheckCamera_ID] "<<light_source_type<<"code light source: "<<code_light_source;


    //Camera name
    QString camera_name = _M_load_datas_hard_drive.get_Camera_name();
    int code_camera = decode_Camera_name(camera_name);
    qDebug()<<"[HAcquisitionDisplay::onCheckCamera_ID] "<<camera_name<<"code camera: "<<code_camera;


    if (code_light_source == -1)
    {
        qDebug()<<"[HAcquisitionDisplay::onCheckCamera_ID] Wrong light source type in Acquisition_info.txt";
        QMessageBox::critical(nullptr, "Wrong entry in Acquisition_info.txt", "Wrong light source type in Acquisition_info.txt");
        QCoreApplication::quit();
        return;
    }


    if (code_camera == -1)
    {
        qDebug()<<"[HAcquisitionDisplay::onCheckCamera_ID] Wrong camera name in Acquisition_info.txt";
        QMessageBox::critical(nullptr, "Wrong entry in Acquisition_info.txt", "Wrong camera name in Acquisition_info.txt");
        QCoreApplication::quit();
        return;
    }



    emit newOpticalConfigIDx(code_camera,code_light_source);
}


//Set HMI mode (user, guru)
void HAcquisitionDisplay::onNew_Guru_Mode(bool v)
{
    ui->_ROI->onNew_Guru_Mode(v);
    if(v)
    {
        ui->_enable_filtering->show();
        ui->_widget_option->show();
        ui->_widget_info->show();
    }
    else
    {
        ui->_enable_filtering->hide();
        ui->_widget_option->hide();
        ui->_widget_info->hide();
    }
}





//Set ROI
void HAcquisitionDisplay::onNewAutoRectROI(QPoint p1, QPoint p2)
{
    qDebug()<<"ROI rect: P1("<<p1.x()<<";"<<p1.y()<<") p2("<<p2.x()<<";"<<p2.y()<<")";
    //Set ROI
    _M_post_acquis.setRectROI(p1,p2);
    emit newRectROI(p1,p2);

    //Send first image to segmentation class
    emit newFirstImg(_M_post_acquis.getPreProcessedImg());


}

void HAcquisitionDisplay::onNewRectROI(QPoint p1, QPoint p2)
{
    qDebug()<<"[HAcquisitionDisplay::onNewRectROI] start";
    //Set ROI
    _M_post_acquis.setRectROI(p1,p2);
}


//get nb of acquisition frames
int HAcquisitionDisplay::getAcquisitionFrameNumber()
{
    int nb_frames=0;

    if(_M_load_datas_hard_drive.isPathLoaded())
        nb_frames = _M_load_datas_hard_drive.getTotalFrameNumber();

    return nb_frames;
}

//Get nb of spectral component
int HAcquisitionDisplay::getSpectralCompenentNumber()
{
    int spectral_components=3;
    return spectral_components;
}

//When data acquisition is ready (when all variables have been initialized)
void HAcquisitionDisplay::onDataAcquisitionIsReady()
{
    //Inform that the rest of the software is ready to store the registred images
    _M_reg.DataAcquisitionIsReady();
}


//Message
int HAcquisitionDisplay::_showInfosMessage(QString text, QString infoText,bool YesNoQuestion)
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


//new analysis zone
void HAcquisitionDisplay::newAnalysisZone(QVector<QPoint> c)
{
    _M_reg.setAnalysisZone(c);
    _startAcquisition();

    ui->_enable_filtering->setEnabled(false);
    ui->_widget_option->setEnabled(false);
    ui->_ROI->setEnabled(false);
}

void HAcquisitionDisplay::_startAcquisition()
{
    _M_require_Learning_First_img = true;

    //Write infos (Pre ROI and spatial sampling in txt file
    _M_post_acquis.WriteInfos();


    if(!_M_load_datas_hard_drive.isPathLoaded())
        return;

    //get nb of frame of the video
    int nb_frames = _M_load_datas_hard_drive.getTotalFrameNumber();

    //Set the number of frames to acquire at the registration class
    _M_reg.setTotalNbFrames(nb_frames);

    //Set transformation model
    _M_reg.setTransfoModel();



    //Get first image
    Mat img = _M_post_acquis.getPreProcessedImg();

    //Send first image to segmentation class
    emit newFirstImg(img);

    //Set first image to registration class
    _SetFirstImg(img);


    //Load datas
    _M_load_datas_hard_drive.LoadDatas();


}

//Request first image to camera and send it to Registration class
void HAcquisitionDisplay::_SetFirstImg(Mat img)
{
    _M_require_Learning_First_img=false;

    if(img.cols==0 || img.rows==0)
        return;

    //Bug!!!!! on doit être paramétrer en reg type tvl1 ou of afin de pouvoir utiliser ARegistration::setFirstImg
    //Sinon crash!!!!
    _M_reg.newRegistrationtype(0);

    //Set First img to Registration class
    _M_reg.setFirstImg(img);

    //Retour au type de reg de base
    _M_reg.newRegistrationtype(2);
}






//Camera mode or load image from hard drive
void HAcquisitionDisplay::onNewAcquisitionMode(bool)
{
    ui->_param_LoadImg->show();

}


//Update data acquisition progress bar
void HAcquisitionDisplay::updateDataAcquisitionProgressBar(int v,int tot)
{
    emit newProgressBarDataAcquisition((int)(v*100)/(tot-1));
}


/** request analysis choice */
void HAcquisitionDisplay::requestAnalysisChoose(QVector<int> v)
{
    _M_choose_analysis.setAnalysis(v);

    this->setEnabled(false);

}

/** enable analysis */
//-1: no analysis
//0 : task-based
//1: resting state
//2: Impulsion
void HAcquisitionDisplay::enableAnalysis(int v)
{
    // No analysis available exit the soft
    if(v == -1)
        QCoreApplication::exit();

    //Enable analysis
    this->setEnabled(true);

    //update HMI
    emit AnalysisChoice(v);

    qDebug()<<"[HAcquisitionDisplay::enableAnalysis]";
    // Request start and end frame for loading images
    _M_load_datas_hard_drive.getStartEndAnalysisFrames(v);
}



