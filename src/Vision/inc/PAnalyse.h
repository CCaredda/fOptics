/**
 * @file PAnalyse.h
 *
 * @brief Main class that schedules all signal and image processing classes.
 *
 *
 * @author Charly Caredda
 * Contact: caredda.c@gmail.com
 *
 */

#ifndef PANALYSE_H
#define PANALYSE_H

#include <QObject>
#include <QImage>
#include <QDir>
#include <QTime>



#include <QDebug>
#include "filtering.h"
#include "conversion.h"
#include "oxygenation.h"

#include <QElapsedTimer>
#include "pobject.h"
#include "acquisition.h"
#include "PFiltering.h"
#include <QTimer>

#include <omp.h>
#include <QProcess>
#include <QThread>
#include "Statistic_functions.h"

#include "PPixelWiseMolarCoeff.h"
#include "PProcessTimes.h"
#include "PModifiedBeerLambertLaw.h"


#include "P_SPM.h"



#define Max_Freq_Cardiac_Filter 2.5
#define Min_Freq_Cardiac_Filter 0.7

#define MAX_data_WEIGHT_GB 11500000000

#define REAL_TIME_PROCESSING_PERIOD 1000



using namespace cv;
using namespace std;

class PAnalyse : public QThread
{
    Q_OBJECT
public:
    explicit PAnalyse(QObject *parent = 0);
    ~PAnalyse();

    /** Check if learning has been done (learning of motion) */
    bool isLearningDone()                   {return _M_learningDone;}
    /** Check if the ROI is empty */
    bool isROIempty()                       {return _M_ROI.empty();}

    /** Set paradigme times (rest and activity periods) */
    void setProcessTimes(QVector<int> v)    {_M_paradigm_times.setProcessTimes(v);}

    /** Set First and last frames for analysis */
    void setFirstLastFrameForAnalysis(int start, int end)   {_M_paradigm_times.setFirstLastFrameForAnalysis(start, end);}


    /** Initialize private variables when methode changed */
    void Initialize();




signals:



    /** Send acquisition info */
    void newAcquisitionInfo(QString);

    /** Emit a new Bold signal */
    void newBoldSignal(QVector<float>);

    /** Emit new activation times */
    void newActivationtimes(QVector<double>);

    /** Inform that Data acquisition is ready (ROI is set and data vector resize) */
    void DataAcquisitionIsReady();

    /** Indicator of the process */
    void newProgressStatut(QString,int);

    /** Computation processing time */
    void Elapsed_ProcessingTime(int);

    /** Emit an error message */
    void Error(QString);

    /** Emit a new concetration changes time courses for plotting */
    void newContrastplot(QVector<QVector<float> >);


    /** Emit a new Contrast image */
    void newContrastImage(QVector<Mat>);


    /** Emit a new pixel pos (position of pixels inside the ROI) */
    void newPixelPos(QVector<Point>);


    /** emit new z threshold calculated by random field theory */
    void newZThresh(double);




public slots:



    /** Get a new max display value */
    void onnewDisplayValue(double v)        {_M_max_display_value = v;}

    /** Get a new initial img */
    void onnewInitialImg(Mat v)             {v.copyTo(_M_initial_img);}

    /** enable or disable filtering (low pass + cardiac + data correction) */
    void onEnableFiltering(bool v)          {_M_filtering.enableFiltering(v);}

    /** enable or disable low pass filtering */
    void onEnableLowPassFiltering(bool v)   {_M_filtering.enableLowPassFiltering(v);}

    /** enable or disable data correction */
    void onEnableDataCorrection(bool v)     {_M_filtering.enableDataCorrection(v);}


    /** Set the MEan ROI Radius */
    void onnewMeanROIRadius(double v)       {_M_Mean_ROI_radius = (int)v;}

    /** Enable the measurement at the level of a ROI */
    void onrequestMeanROIMeasure(bool v)    {_M_request_Mean_ROI_Measure = v;}

    /** Request results saving */
    void onrequestSaveResults(_data_saving_info info);

    /** Request analysis processing */
    void LaunchProcess();


    /*****************************/
    /*********Statistics**********/
    /*****************************/


    /** on new stat type */
    void onnewStatType(int v);

    /** on new FHWM value (used in RTF to smooth the data the Gaussian fitering) */
    void onnewFHWM(double v)                    {_M_SPM.setFWHM(v);}

    /** On new level of statistical significance */
    void onnewStatisticalLevel(int v)           {_M_SPM.set_Statistical_Significance_Level(v);}

    /** Set number of resels */
    void onNewReselsNumber(int r);


    /*****************************/
    /*********Camera config*******/
    /*****************************/


    /** new optical device ID */
    void onNewOpticalConfigIDx(int camera_id, int light_id)   {_M_PixelWise_Molar_Coeff.setOpticalConfigIDx(camera_id, light_id);}


    /*****************************/
    /*********New Images**********/
    /*****************************/

    /** Send RGB data to the PFiltering class */
    void addDatas(_Processed_img &img);




    /** Set Camera Frame rate */
    void setFrameRate(double v)                    {_M_frame_rate=v;_M_filtering.setFrameRate(v);_M_paradigm_times.setFrameRate(v);}

    /** Set Region of interest */
    void setAnalysisZone(QVector<QPoint> c, Size img_size, int nbChannels, int NbFrames);

    /** Set Point of interest (clicked on graphic interface) */
    void setStudiedPoint(Point P);

    /** Inform Data finish loading (for not Real time Process) */
    void DataFinishLoading();


    /** Set chromophore id: HbO2, Hb, (oxCCO), HbT */
    void onNewChromophoreID(int v)              {_M_chromophore_id = v; }

    /** Indicate the number of activation step to consider for the calculation of correlation and SPM */
    void onNewActivationStepsToConsider(int v)           {_M_paradigm_times.setActivationStepsToConsider(v); LaunchProcess();}


    /** New result directory */
    void onNewResultDirectory(QString v);

protected:
    /** Call process in parallel thread */
  void run();

private:
    //Private function

    //Get mean contrast averaged over a disk
    void _Get_Stats_Mask_pos(Mat mask,QVector<int> &pos);
    void _getMeanContrast(QVector<QVector<float> > &mean_contrast,QVector<int> id,QString write_file,int rest_start,int rest_end,int correlation_start,int correlation_end);
    void _getMeanROIMeasurement(Point P,int rest_start,int rest_end,int correlation_start,int correlation_end,QVector<QVector<float> > &mean_contrast,QString saving_dir);



    // Get Delta C maps for statistical tests
    QVector<Mat> _Get_Delta_C_Maps(int nb_chromopores);

    //Get camera intensities
    QVector<Mat> _Get_Camera_Intensity();


    //Save results
    void _Save_Results();

    //Process statistics per zone
    void _General_Linear_Model_Random_Field_Theory();
    void _General_Linear_Model_Auto_thresh();



    //Write Img frame (for img clustering)
    void  _WriteImg(Mat &img);

    //Process mean concentration measure
    void _ProcessMeanConcentrationMeasure();
    QVector<Mat> _Process_Mean_Delta_C();

    //Process Correlation of BOLD signal
    void _ProcessCorrelationMeasure();
    void _ProcessCorrelation(QVector<QVector<float> > &Concentration_map);


    //Display oxynation concentration results
    Mat _Display(QVector<float> &contrast_proc);
    Mat _Display(QVector<bool> &contrast_proc);
    Mat _Display(Mat &contrast_proc);
    Mat _Display(const Mat &contrast_proc,const QVector<int> &id);

    //Write img result
    Mat _writeCartography(Mat &contrast_proc, QString filename);
    void _writeFloatCartography(Mat &contrast_proc,QString filename);

    //reconstruct img
    Mat _reconstruct_Float_Carography(Mat &contrast_proc);
    Mat _reconstruct_uchar_Carography(Mat &contrast_proc);

    //Create falsecolormap
    Mat _Convert_to_FalseColor(Mat &in, Mat &mask_img, double &min, double &max);
    Mat _create_colorbar(Mat in_img);

    //Draw text on image
    Mat _drawTextOnImg(Mat img, String text, int pos, double font_size, int thick);




    int                         _M_point_of_interest;

    //Data info
    int                         _M_tot_frames;
    int                         _M_nb_channels;
    int                         _M_nb_temporal_vectors;
    double                      _M_frame_rate;


    //ROI
    vector<Point>               _M_ROI;

    //Pixel pos
    QVector<cv::Point>          _M_pixels_pos;

    //Indicators (Learning, acquisition, processing)
    bool                        _M_learningDone;

    //Video frames size
    cv::Size                    _M_img_size;


    PPixelWiseMolarCoeff        _M_PixelWise_Molar_Coeff;
    PModifiedBeerLambertLaw     _M_MBLL;


    //Statistical Parametric Mapping
    P_SPM                        _M_SPM;


    //Processing times in not RT mode (reference times and process times)
    PProcessTimes               _M_paradigm_times;


    //Fitlering class
    PFiltering                  _M_filtering;

    //Statistics
    bool                        _M_get_filtered_non_filtered_signals;

    //Processing type (SPM, t-tests, Mean concentration changes, correlation....
    int                         _M_processing_type;


    //Save resutls
    bool                        _M_save_results;
    _data_saving_info           _M_saving_info;


    //Radius
    int _M_Mean_ROI_radius;

    //Enable measurement
    bool _M_request_Mean_ROI_Measure;


    //Max display value
    double _M_max_display_value;

    //Create video
    Mat     _M_initial_img;


    //chromophore ID (HbO2, Hb, (oxCCO))
    int _M_chromophore_id;



    //Contrast idx for phase analysis
    //0: HbO2; 1: Hb; 2: HbT; 3: oxCCO
    int _M_id_phase1;
    int _M_id_phase2;

    //Result directory
    QString _M_result_directory;


    //Hyperspectral img dynamic
    int _M_HS_Dyn;


};

#endif // PANALYSE_H
