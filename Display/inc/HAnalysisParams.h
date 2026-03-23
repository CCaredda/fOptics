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

    /** Set HMI mode (user, guru) */
    void onNew_Guru_Mode(bool);


private slots:
    /** Set elapsed process time */
    void SetElapsed_ProcessTime(int);

    /** Error message */
    void onNewErrorMessage(QString);


public slots:

    /** Analysis choice (from acquisition) */
    void AnalysisChoice(int);


    //Statistic mask
    /** new activated cortical area definition */
    void onnewCorticalAreaDefinition(Rect,QVector<Rect>,QVector<Mat>);
    /** new activated cortical area definition */
    void onnewCorticalAreaDefinition(QVector<Rect>,QVector<Mat>);

    /** New process image (RGB) */
    void onNewProcessImage(_Processed_img);
    /** New process image (Hyperspectral) */
    void onNewProcessImage_HS(_Processed_img_HS);

    /** Analysis zone requested */
    void onAnalysisZoneRequested(int v);

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

    /** resting state seeds */
    void onnewRestingStateSeeds(QVector<Mat> v)     {_M_analyse.onnewRestingStateSeeds(v);}
    /**  Init resting state seeds */
    void InitRestingStateSeeds()                    {_M_analyse.InitRestingStateSeeds();}

    /** New resting state method (0: seed based method, 1: ICA based method) */
    void onnewRestingStateMethod(int v)             {_M_analyse.onnewRestingStateMethod(v);}

    /** new nb of independant sources (ICA) */
    void onnewNbofIndependantSources_ICA(double v)  {_M_analyse.onNewIndependantNbofSources((int)v);}

    /** On new max display value */
    void onnewDisplayValue(double v)                {_M_analyse.onnewDisplayValue(v);}

    /** on new initial img */
    void onnewInitialImg(Mat v)                     {_M_analyse.onnewInitialImg(v);}

    /** grey outside contour */
    void onGreyOutsideContourIsRequested(bool v)    {_M_analyse.onGreyOutsideContourIsRequested(v);}

    /** set result directory */
    void onNewResultDirectory(QString);

signals:



    /** New z thresh (RFT threshold) */
    void newZThreshValue(double);

    /** Request acquisition info */
    void requestAcquisitionInfo(bool);

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



    /** New Hyperspectral image */
    void newFirstHSImage(vector<Mat>);

    /** New Mean ROI */
    void newMeanROIRadius(double);
    /** Request a mean ROI */
    void requestMeanROIMeasure(bool);

    /** Request save results */
    void requestSaveResults(_data_saving_info);

    /** Enable or disable option for saving non filtered signals */
    void requestNonFilteredDataSaving(bool);

    /** New Segmentation layers */
    void newSegmentationLayers(Mat,Mat,Mat);

    /** Consider Blood vessel distance */
    void ConsiderBloodVesselDistance(bool);

    /** New hyperspectral segmentation */
    void newHyperspectralSegmentation(QVector<int>);

    /** New setting for SPM : Full half width of the Gaussian kernel */
    void newSettingStatisticZone(double);




    /** New Hyperspectral cam config */
    void new_HS_config(int);

    /** New hyperspectral spectral range */
    void newSpectralRange(int,int);

    /** new optical device id */
    void newOpticalConfigIDx(int,int);

    /** Require spectral correction */
    void requireSpectralCorrection(bool);



    /** resquest image clearing */
    void ClearImgRequested();

    /** Request analysis zone definition (0: Drawing, 4: Rect) */
    void AnalysisZoneRequested(int);

    /** Inform that data acquisition is ready */
    void DataAcquisitionIsReady();

    /** new Progress value */
    void newProgressValue(QString,int);
    /** new RT progress value */
    void newRTProgressValue(QString,int);

    /** new contrast image */
    void newContrastImage(QVector<Mat>);
    /** RT cartography */
    void newRTCartography(Mat,int);

    /** New Activation map */
    void newActivationMap(QVector<bool>);
    /** New Activation map */
    void newActivationMap(QVector<bool>,int);
    /** New Activation map */
    void newActivationMap(Mat);
    /** New Activation map */
    void newActivationMap(QVector<Mat>);

    /** New activation statistics setting */
    void newStatType(int);


    /** new image cluster */
    void newImageCluster(Mat);

    /** new methode auto (true) or beer lambert (false) */
    void newMethodeAuto(bool);

    /** new contrast point */
    void newContrastplot(QVector<QVector<float> >);

    /** new FFT vectors */
    void newFFTVectors(QVector<QVector<float> >);

    /** new filter */
    void newFilter(QVector<float>);

    /** new correlation threshold */
    void newCorrelationThreshold(double);


    /** New pixel position */
    void newPixelPos(QVector<Point>);

    /** new chromophore ID */
    void newChromophoreID(int);


    /** Apply Resting state grid seed */
    void newGridSeeds(bool);
    /** Set resting state seed radius */
    void newSeedRadius(double);
    /** Request seed init */
    void requestSeedInit();
    /** Enable resting state */
    void enableRestingState(bool);
    /** Request seeds */
    void requestSeeds();
    /** New resting state maps */
    void newRestingStateMaps(QVector<QVector<Mat> >);
    /** New K-Means resting state maps */
    void newKmeans_RestingStateMaps(QVector<Mat>);
    /** Request seed connexion */
    void requestSeedConnexion(bool);
    /** Request seed extraction */
    void requestSeedsExtraction();
    /** New resting state method (seed, ICA, Kmeans) */
    void newRestingStateMethod(int);
    /** New ICA source */
    void newICASourceofInterest(int);

    /** Request video creation */
    void VideoCreationRequired();

    /** Number of frames in the video */
    void newVideo_nb_Frames(double);

    /** video framerate */
    void newVideoFramerate(double);



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

    //learning OK
    bool    _M_motion_learned;

    //Data ready
    bool    _M_data_ready;
};

#endif // HANALYSISPARAMS_H
