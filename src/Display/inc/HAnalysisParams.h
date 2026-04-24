/**
 * @file HAnalysisParams.h
 *
 * @brief This class contains the graphical interface classes to compute image analyses (signal processing, statistics and video saving)
 *
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef HANALYSISPARAMS_H
#define HANALYSISPARAMS_H

#include <QWidget>
#include "PAnalyse.h"
#include <QMessageBox>
#include <QtConcurrent>


namespace Ui {
class HAnalysisParams;
}

class HAnalysisParams : public QWidget
{
    Q_OBJECT

public:
    explicit HAnalysisParams(QWidget *parent = 0);
    ~HAnalysisParams();




private slots:
    /** Set elapsed process time */
    void SetElapsed_ProcessTime(int);



    /** Error message */
    void onNewErrorMessage(QString);


public slots:

    /** Analysis choice (from acquisition) */
    void AnalysisChoice(int);


    //Statistic mask

    /** New process image (RGB) */
    void onNewProcessImage(_Processed_img);


    /** Analysis zone requested */
    void onAnalysisZoneRequested();

    /** New analysis zone received */
    void onNewAnalysisZone(QVector<QPoint>, Size, int nb_channels, int nb_frames);

    /** Launch process */
    void onLaunchProcess();

    /** new frame rate value */
    void onNewFrameRate(double v);

    /** New process times */
    void onNewProcessTimes(QVector<int>,double Fs); //(define by camera acquistion)

    /** Set first and last frames for analysis */
    void onNewFirstLastAnalysisFrames(int,int);

    /** On point selected on the image */
    void onPointSelected(Point);

    /** On new max display value */
    void onnewDisplayValue(double v)                {_M_analyse.onnewDisplayValue(v);}

    /** on new initial img */
    void onnewInitialImg(Mat v)                     {_M_analyse.onnewInitialImg(v);}

    /** set result directory */
    void onNewResultDirectory(QString);

signals:

    /** New resels number */
    void newReselsNumber(int);

    /** New z thresh (RFT threshold) */
    void newZThreshValue(double);


    /** Send acquisition info */
    void newAcquisitionInfo(QString);

    /** Activation times */
    void newActivationtimes(QVector<double>);

    /** Bold signal */
    void newBoldSignal(QVector<float>);

    /** Enable filtering */
    void enableFiltering(bool v);
    /** Enable data correction */
    void enableDataCorrection(bool v);
    /** Enable low pass filtering */
    void enableLowPassFiltering(bool v);


    /** New Mean ROI */
    void newMeanROIRadius(double);
    /** Request a mean ROI */
    void requestMeanROIMeasure(bool);

    /** Request save results */
    void requestSaveResults(_data_saving_info);


    /** New setting for SPM : Full half width of the Gaussian kernel */
    void newSettingStatisticZone(double);


    /** new optical device id */
    void newOpticalConfigIDx(int,int);


    /** resquest image clearing */
    void ClearImgRequested();

    /** Request analysis zone definition */
    void AnalysisZoneRequested();

    /** Inform that data acquisition is ready */
    void DataAcquisitionIsReady();

    /** new Progress value */
    void newProgressValue(QString,int);


    /** new contrast image */
    void newContrastImage(QVector<Mat>);



    /** New activation statistics setting */
    void newStatType(int);



    /** new contrast point */
    void newContrastplot(QVector<QVector<float> >);



    /** New pixel position */
    void newPixelPos(QVector<Point>);

    /** new chromophore ID */
    void newChromophoreID(int);




private:
    Ui::HAnalysisParams *ui;

    /** Display message */
    int _showInfosMessage(QString text, QString infoText,bool YesNoQuestion);

    //Data analysis class
    PAnalyse    _M_analyse;

    QVector<Mat> _M_stats_zone;

    //Nb of frames
    int _M_nb_frames;

    //Frame per second
    int _M_FS;


};

#endif // HANALYSISPARAMS_H
