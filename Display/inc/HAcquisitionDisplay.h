/**
 * @file HAcquisitionDisplay.h
 *
 * @brief This class contains the graphical interface classes to control data acquisition (acquire images from a camera if a camera
 * is detected or load images from hardrive) and to preprocess images.
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HACQUISITIONDISPLAY_H
#define HACQUISITIONDISPLAY_H


#include <QWidget>
#include <QMessageBox>
#include "AloadDatas.h"
#include "acquisition.h"
#include "APostAcquisition.h"

#include "AImageRegistration.h"
#include <QTimer>

#include "HChooseAnalysis.h"
#include <thread>
#include <chrono>

namespace Ui {
class HAcquisitionDisplay;
}

class HAcquisitionDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit HAcquisitionDisplay(QWidget *parent = 0);
    ~HAcquisitionDisplay();

    /** get nb of acquisition frames */
    int getAcquisitionFrameNumber();

    /** Get nb of spectral component */
    int getSpectralCompenentNumber();

    /** Set HMI mode (user, guru) */
    void onNew_Guru_Mode(bool);

    /** Get result directory */
    QString getResultDirectory()    {return _M_result_directory;}

private slots:


    /** on new RGB mode */
    void onnewRGBMode(bool);

    /** Camera mode or load image from hard drive */
    void onNewAcquisitionMode(bool);

    /** Update data acquisition progress bar */
    void updateDataAcquisitionProgressBar(int, int tot);



    //Check camera ID (in metadata)
    void onCheckCamera_ID();


    /** request analysis choice */
    void requestAnalysisChoose(QVector<int>);

    /** enable analysis */
    //-1: no analysis
    //0 : task-based
    //1: resting state
    //2: Impulsion
    void enableAnalysis(int);

public slots:
    /** New acquisition info */
    void onNewAcquisitionInfo(QString msg);

    /** New automatic rect ROI */
    void onNewAutoRectROI(QPoint,QPoint);
    /** New manual rect ROI */
    void onNewRectROI(QPoint, QPoint);

    /** new analysis zone */
    void newAnalysisZone(QVector<QPoint>);

   /** When data acquisition is ready (when all variables have been initialized) */
    void onDataAcquisitionIsReady();

private slots:
    /** New result directory */
    void onNewResultDirectory(QString);

signals:

    /** Analysis choice
    -1: no analysis
    0 : task-based
    1: resting state
    2: Impulsion */
    void AnalysisChoice(int);

    /** New result directory */
    void newResultDirectory(QString);






    /*******************************/
    /****Acquisition signals*****/
    /******************************/

    /** First and last frames for analysis */
    void newFirstLastAnalysisFrames(int,int);

    /** Request acquisition info */
    void requestAcquisitionInfo(bool);

    /** Spectral range (hyperspectral imaging) */
    void newSpectralRange(int,int);

    /** Require spectral correction */
    void requireSpectralCorrection(bool);

    /** new optical device id */
    void newOpticalConfigIDx(int,int);

    /*******************************/
    /****Pre-processing signals*****/
    /******************************/


    /** Enable filtering */
    void enableFiltering(bool v);
    /** Enable data correction */
    void enableDataCorrection(bool v);
    /** Enable data low pass filtering */
    void enableLowPassFiltering(bool v);


    /***********************/
    /****Image* signals*****/
    /***********************/

    /** new img displayed */
    void newDisplayImage(Mat);
    /**  New initial image displayed */
    void newInitialImg(Mat);

    /** New processed img (RGB) */
    void newProcessImage(_Processed_img);
    /** New processed img (HS) */
    void newProcessImage_HS(_Processed_img_HS);

    /** Clear img requested */
    void ClearImgRequested();


    /** new rect roi */
    void newRectROI(QPoint,QPoint);


    /** new automatic analysis zone */
    void newAutomaticAnalysisZone(QVector<QPoint>,cv::Size);

    /** request analysis zone (0: drawing, 4: ROI) */
    void requestAnalysisZoneDrawing(int);

    /** new HS config */
    void new_HS_config(int);
    /** New RGB mode */
    void newRGBMode(bool);

    /** Request ROI selection */
    void onRectROIRequested();

    /** new acquisition progress bar values */
    void newAcquisitionProcess(QString,int);
    void newProgressBarDataAcquisition(int);

    /** New frame rate value */
    void newFrameRate(double);

    /** New process times */
    void newProcessTimes(QVector<int>,double);

    /** Send first image to Clustering class (RGB) */
    void newFirstImg(Mat);
    /** Send first image to Clustering class (HS) */
    void newFirstImg(vector<Mat>);

    /***********************/
    /****Camera signals*****/
    /***********************/

    /** request line plot */
    void RequestLinePlot();

    /** New line */
    void newLine(QPoint,QPoint);



private:
    Ui::HAcquisitionDisplay *ui;



    //Set first img to Registration class
    void _SetFirstImg(Mat img);
    void _SetFirstImg(vector<Mat> img);

    //Message
    int _showInfosMessage(QString text, QString infoText,bool YesNoQuestion);


//    //Learn registration Model
//    void LearnRegistrationModel_RGB();
//    void LearnRegistrationModel_HS();

    //start acquisition
    void _startAcquisition();

    //Load data from hard drive
    AloadDatas                      _M_load_datas_hard_drive;

    //Post acquisition
    APostAcquisition                _M_post_acquis;

    //Registration class
    AImageRegistration              _M_reg;

    //Require First img
    bool                            _M_require_Learning_First_img;


    //Choose analysis when selecting directory
    HChooseAnalysis                 _M_choose_analysis;


    //Result directory
    QString _M_result_directory;

//    //TEMP IMG
//    Mat temp_img;

};

#endif // HACQUISITIONDISPLAY_H
